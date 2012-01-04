// Copyright 2012 Google Inc.
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
//
// Declares the ExplodeBasicBlocksTransform. This transform seperates all of
// the basic-blocks in a block-graph into individual code and data blocks.
// This is primarily a test to exercise the basic-block motion machinery.

#ifndef SYZYGY_PE_TRANSFORMS_EXPLODE_BASIC_BLOCKS_TRANSFORM_H_
#define SYZYGY_PE_TRANSFORMS_EXPLODE_BASIC_BLOCKS_TRANSFORM_H_

#include "base/file_path.h"
#include "syzygy/block_graph/transforms/named_transform.h"

namespace pe {
namespace transforms {

// A sample BlockGraph transform that explodes all basic-blocks in each code
// block into individual code or data blocks.
class ExplodeBasicBlocksTransform
    : public block_graph::transforms::NamedTransformImpl<
          ExplodeBasicBlocksTransform> {
 public:
  typedef block_graph::BlockGraph BlockGraph;

  ExplodeBasicBlocksTransform();

  // Seperates all basic blocks in the given block graph into individual code
  // and data blocks.
  //
  // @param block_graph The block graph to transform.
  // @param dos_header_block The DOS header block of the block graph.
  // @returns true on success, false otherwise.
  virtual bool Apply(BlockGraph* block_graph,
                     BlockGraph::Block* dos_header_block) OVERRIDE;

  // The tranform name.
  static const char kTransformName[];

 private:

  DISALLOW_COPY_AND_ASSIGN(ExplodeBasicBlocksTransform);
};

}  // namespace transforms
}  // namespace pe

#endif  // SYZYGY_PE_TRANSFORMS_EXPLODE_BASIC_BLOCKS_TRANSFORM_H_
