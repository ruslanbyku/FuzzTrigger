#include "cfg.h"

CFG::CFG(const llvm::Function& function)
: CFG(function, function.getBasicBlockList().size()) {}

CFG::CFG(const llvm::Function& function, uint64_t blocks_number)
: function_(function) {
    // Reserve space for each basic block - vertex
    adjacency_list_.reserve(blocks_number);

    // Initialize adjacency list for vertices
    for (uint64_t idx = 0; idx < blocks_number; ++idx) {
        adjacency_list_.emplace_back(List());
    }
}

void CFG::AddVertex(uint32_t block_id, const llvm::BasicBlock* basic_block) {
    Vertex vertex;
    vertex.id_    = block_id;
    vertex.block_ = basic_block;

    adjacency_list_[block_id][vertex];
}

void CFG::AddEdge(const llvm::BasicBlock* from, const llvm::BasicBlock* to) {
    const Vertex& U = GetVertexByBlock(from);
    const Vertex& V = GetVertexByBlock(to);

    adjacency_list_[U.id_][U].push_back(V);
}

const llvm::Function& CFG::GetFunction() const {
    return function_;
}

const AdjacencyList& CFG::GetAdjacencyList() const {
    return adjacency_list_;
}

std::string CFG::GetFunctionName() const {
    return function_.getName().str();
}

const Vertex& CFG::GetVertexByBlock(const llvm::BasicBlock* basic_block) {
    //TODO: resolve the situation when there is no particular object
    for (auto& map_: adjacency_list_) {
        for (auto& pair: map_) {
            if (pair.first.block_ == basic_block) {
                return pair.first;
            }
        }
    }
}
