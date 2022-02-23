#ifndef AUTOFUZZ_CFG_H
#define AUTOFUZZ_CFG_H

#include <cstdint>
#include <vector>
#include <map>

#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>

struct Vertex {
    virtual ~Vertex() = default;

    uint32_t id_ = 0;
};

template <typename T>
struct VertexData : Vertex {
    ~VertexData() override = default;

    const T* object_ = nullptr;
};

using VertexPtr = std::shared_ptr<Vertex>;
using LinkedList = std::vector<VertexPtr>;

using Row = std::map<VertexPtr, LinkedList,
        decltype([](const VertexPtr& U, const VertexPtr& V) -> bool {
            return U->id_ < V->id_;
        })>;
using AdjacencyList = std::vector<Row>;


template <typename T>
class CFG {
public:
    explicit CFG()             = default;
    virtual ~CFG()             = default;
    CFG(const CFG&)            = delete;
    CFG& operator=(const CFG&) = delete;
    CFG(CFG&&)                 = delete;
    CFG& operator=(CFG&&)      = delete;

    // Add new node (vertex) to the adjacency list
    // This method is called to initialize (find) all vertices in the function
    void AddVertex(uint32_t id, const T* object) {
        VertexPtr vertex     = std::make_shared<VertexData<T>>();
        auto vertex_data     = static_cast<VertexData<T>*>(vertex.get());
        vertex_data->id_     = id;
        vertex_data->object_ = object;

        // If there is a shortage in rows, append a new one
        bool has_row = id < adjacency_list_.size();
        if (!has_row) {
            InitializeRow();
        }

        adjacency_list_[id][std::move(vertex)];
    }

    // Add new edge E(u, v) to the adjacency list
    // This method is called when a new neighbor vertex of a vertex was found
    void AddEdge(const T* from, const T* to) {
        VertexPtr U = GetVertexByObject(from);
        if (U == nullptr) {
            return;
        }

        VertexPtr V = GetVertexByObject(to);
        if (V == nullptr) {
            return;
        }

        adjacency_list_[U->id_][U].push_back(V);
    }

    VertexPtr GetVertexByObject(const T* object) const {
        for (const auto& row: adjacency_list_) {
            for (const auto& pair: row) {
                const auto vertex_data =
                        static_cast<VertexData<T>*>(pair.first.get());
                if (vertex_data->object_ == object) {
                    return pair.first;
                }
            }
        }

        return nullptr;
    }

    const AdjacencyList& GetAdjacencyList() const {
        return adjacency_list_;
    }

protected:
    AdjacencyList adjacency_list_;

private:
    inline void InitializeRow() {
        adjacency_list_.emplace_back(Row());
    }

};

class BasicBlockCFG : public CFG<llvm::BasicBlock> {
public:
    explicit BasicBlockCFG(const llvm::Function& function)
               : BasicBlockCFG(function, function.getBasicBlockList().size()) {}
    explicit BasicBlockCFG(const llvm::Function& function,
                           uint64_t blocks_number)
                               : CFG<llvm::BasicBlock>(), function_(function) {
        // Reserve space for each basic block - vertex
        adjacency_list_.reserve(blocks_number);

        // Initialize adjacency list for vertices
        for (uint64_t idx = 0; idx < blocks_number; ++idx) {
            adjacency_list_.emplace_back(Row());
        }
    }

    ~BasicBlockCFG() override                      = default;
    BasicBlockCFG(const BasicBlockCFG&)            = delete;
    BasicBlockCFG& operator=(const BasicBlockCFG&) = delete;
    BasicBlockCFG(BasicBlockCFG&&)                 = delete;
    BasicBlockCFG& operator=(BasicBlockCFG&&)      = delete;

    const llvm::Function& GetFunction() const {
        return function_;
    }

private:
    const llvm::Function& function_;
};

class FunctionCFG : public CFG<llvm::Function> {
public:
    explicit FunctionCFG() : CFG<llvm::Function>() {}
    ~FunctionCFG() override                    = default;
    FunctionCFG(const FunctionCFG&)            = delete;
    FunctionCFG& operator=(const FunctionCFG&) = delete;
    FunctionCFG(FunctionCFG&&)                 = delete;
    FunctionCFG& operator=(FunctionCFG&&)      = delete;
};

#endif //AUTOFUZZ_CFG_H