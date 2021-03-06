// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "syzygy/pe/transforms/coff_rename_symbols_transform.h"

#include <string>
#include <vector>

#include "base/bind.h"
#include "syzygy/block_graph/typed_block.h"
#include "syzygy/pe/coff_utils.h"

namespace pe {
namespace transforms {

namespace {

using block_graph::BlockGraph;
using block_graph::TypedBlock;

void AddSymbol(const base::StringPiece& symbol_name,
               BlockGraph::Offset template_offset,
               BlockGraph::Block* symbols_block,
               BlockGraph::Block* strings_block,
               BlockGraph::Offset* symbol_offset) {
  DCHECK_NE(reinterpret_cast<BlockGraph::Block*>(NULL), symbols_block);
  DCHECK_NE(reinterpret_cast<BlockGraph::Block*>(NULL), strings_block);
  DCHECK_NE(reinterpret_cast<BlockGraph::Offset*>(NULL), symbol_offset);

  TypedBlock<IMAGE_SYMBOL> symbols;
  CHECK(symbols.Init(0, symbols_block));
  size_t symbol_count = symbols.ElementCount();
  *symbol_offset = sizeof(IMAGE_SYMBOL) * symbol_count;
  symbols_block->InsertData(*symbol_offset, sizeof(IMAGE_SYMBOL), true);
  size_t template_index = template_offset / sizeof(IMAGE_SYMBOL);
  IMAGE_SYMBOL* orig = &symbols[template_index];
  IMAGE_SYMBOL* symbol = &symbols[symbol_count];

  // Copy the metadata from the template symbol. We set the section number to
  // zero to indicate that this is an external symbol that has no definition in
  // this COFF file. It will be satisfied at link time.
  symbol->Value = orig->Value;
  symbol->SectionNumber = 0;
  symbol->Type = orig->Type;
  symbol->StorageClass = orig->StorageClass;
  symbol->NumberOfAuxSymbols = 0;

  // Determine whether the name goes in the string table or is embedded in the
  // symbol record itself.
  char* symbol_name_dst = NULL;
  if (symbol_name.size() <= sizeof(symbol->N.ShortName)) {
    symbol_name_dst = reinterpret_cast<char*>(symbol->N.ShortName);
  } else {
    size_t string_offset = strings_block->size();
    strings_block->set_size(strings_block->size() + symbol_name.size() + 1);
    strings_block->ResizeData(strings_block->size());
    symbol_name_dst = reinterpret_cast<char*>(
        strings_block->GetMutableData()) + string_offset;
    symbol->N.Name.Long = string_offset;
  }

  // Copy the symbol name. We don't explicitly copy the terminating NULL, as
  // the data structure was initialized with zeros and we don't always need one
  // (the case of an 8-byte name, which is stored directly in the symbol).
  ::memcpy(symbol_name_dst, symbol_name.data(), symbol_name.size());

  return;
}

void TransferReferrers(BlockGraph::Offset src_offset,
                       BlockGraph::Offset dst_offset,
                       BlockGraph::Block* block) {
  DCHECK_NE(reinterpret_cast<BlockGraph::Block*>(NULL), block);

  // Make a copy of the referrers set because we'll be modifying the original
  // as we traverse.
  BlockGraph::Block::ReferrerSet referrers = block->referrers();
  BlockGraph::Block::ReferrerSet::const_iterator ref_it = referrers.begin();
  for (; ref_it != referrers.end(); ++ref_it) {
    BlockGraph::Reference ref;
    CHECK(ref_it->first->GetReference(ref_it->second, &ref));
    DCHECK_EQ(block, ref.referenced());
    if (ref.offset() != src_offset)
      continue;

    BlockGraph::Offset delta = ref.base() - ref.offset();
    ref = BlockGraph::Reference(ref.type(), ref.size(), ref.referenced(),
                                dst_offset, dst_offset + delta);
    CHECK(!ref_it->first->SetReference(ref_it->second, ref));
  }
}

}  // namespace

const char CoffRenameSymbolsTransform::kTransformName[] =
    "CoffRenameSymbolsTransform";

void CoffRenameSymbolsTransform::AddSymbolMapping(const base::StringPiece& from,
                                                  const base::StringPiece& to) {
  mappings_.push_back(std::make_pair(from.as_string(), to.as_string()));
}

bool CoffRenameSymbolsTransform::TransformBlockGraph(
    const TransformPolicyInterface* policy,
    BlockGraph* block_graph,
    BlockGraph::Block* /* headers_block */) {
  DCHECK_NE(reinterpret_cast<TransformPolicyInterface*>(NULL), policy);
  DCHECK_NE(reinterpret_cast<BlockGraph*>(NULL), block_graph);
  DCHECK_EQ(BlockGraph::COFF_IMAGE, block_graph->image_format());

  BlockGraph::Block* symbols_block;
  BlockGraph::Block* strings_block;
  if (!FindCoffSpecialBlocks(block_graph,
                             NULL, &symbols_block, &strings_block)) {
    LOG(ERROR) << "Block graph is missing some COFF special blocks. "
               << "Not a COFF block graph?";
    return false;
  }

  CoffSymbolNameOffsetMap symbol_offset_map;
  if (!BuildCoffSymbolNameOffsetMap(symbols_block, strings_block,
                                    &symbol_offset_map)) {
    return false;
  }

  for (size_t i = 0; i < mappings_.size(); ++i) {
    const std::string& src = mappings_[i].first;
    const std::string& dst = mappings_[i].second;
    CoffSymbolNameOffsetMap::const_iterator src_it =
        symbol_offset_map.find(src);
    if (src_it == symbol_offset_map.end()) {
      if (symbols_must_exist_) {
        LOG(ERROR) << "Unable to find source symbol \"" << src << "\".";
        return false;
      }

      // Input symbols aren't forced to exist, so continue on to the next one.
      continue;
    }
    DCHECK(!src_it->second.empty());

    // Find the destination offset.
    CoffSymbolNameOffsetMap::const_iterator dst_it =
        symbol_offset_map.find(dst);
    BlockGraph::Offset dst_offset = 0;
    if (dst_it != symbol_offset_map.end()) {
      // If the destination is multiply defined we simply take the first one.
      DCHECK(!dst_it->second.empty());
      dst_offset = *dst_it->second.begin();
    } else {
      // If the symbol does not exist, then append it to the strings block.
      // Use the first symbol as canonical for the purpose of symbol metadata.
      BlockGraph::Offset src_offset = *src_it->second.begin();
      AddSymbol(dst, src_offset, symbols_block, strings_block,
                &dst_offset);
    }

    // Iterate over all source symbols with this name and transfer references
    // from them to the destination symbol.
    const CoffSymbolOffsets& src_offsets = src_it->second;
    CoffSymbolOffsets::const_iterator src_offset_it = src_offsets.begin();
    for (; src_offset_it != src_offsets.end(); ++src_offset_it) {
      BlockGraph::Offset src_offset = *src_offset_it;
      TransferReferrers(src_offset, dst_offset, symbols_block);
    }
  }

  return true;
}

}  // namespace transforms
}  // namespace pe
