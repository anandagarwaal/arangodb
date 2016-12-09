////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_PREGEL_GRAPH_STORE_H
#define ARANGODB_PREGEL_GRAPH_STORE_H 1

#include <cstdint>
#include <cstdio>
#include <set>
#include "Cluster/ClusterInfo.h"
#include "Pregel/Graph.h"
#include "Pregel/GraphFormat.h"
#include "Pregel/Iterators.h"

struct TRI_vocbase_t;

namespace arangodb {
class Transaction;
class LogicalCollection;
namespace pregel {
  
class WorkerState;
template <typename V, typename E>
struct GraphFormat;

////////////////////////////////////////////////////////////////////////////////
/// @brief carry graph data for a worker job
////////////////////////////////////////////////////////////////////////////////
template <typename V, typename E>
class GraphStore {
  
  VocbaseGuard _vocbaseGuard;
  const std::unique_ptr<GraphFormat<V, E>> _graphFormat;
  Transaction *_transaction;// temporary transaction
  
  // int _indexFd, _vertexFd, _edgeFd;
  // void *_indexMapping, *_vertexMapping, *_edgeMapping;
  // size_t _indexSize, _vertexSize, _edgeSize;
  // std::map<std::string, std::string> _shardsPlanIdMap;

  // only for demo, move to memory
  std::vector<VertexEntry> _index;
  std::vector<V> _vertexData;
  std::vector<Edge<E>> _edges;

  std::set<ShardID> _loadedShards;
  size_t _localVerticeCount;
  size_t _localEdgeCount;
  
  void _cleanupTransactions();
  void _loadVertices(WorkerState const& state,
                     ShardID const& vertexShard,
                     ShardID const& edgeShard);
  void _loadEdges(WorkerState const& state,
                  ShardID const& shard,
                  VertexEntry& vertexEntry,
                  std::string const& documentID);

 public:
  GraphStore(TRI_vocbase_t* vocbase, WorkerState const& state,
             GraphFormat<V, E>* graphFormat);
  ~GraphStore();

  void loadShards(WorkerState const& state);
  inline size_t vertexCount() {
    return _index.size();
  }
  RangeIterator<VertexEntry> vertexIterator();
  RangeIterator<VertexEntry> vertexIterator(size_t start, size_t count);
  RangeIterator<Edge<E>> edgeIterator(VertexEntry const* entry);

  void* mutableVertexData(VertexEntry const* entry);
  V copyVertexData(VertexEntry const* entry);
  void replaceVertexData(VertexEntry const* entry, void* data, size_t size);
  
  /// Write results to database
  void storeResults(WorkerState const& state);
};
}
}
#endif