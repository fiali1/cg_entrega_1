#include <fmt/core.h>

#include <imgui.h>

#include <cppitertools/itertools.hpp>

#include "abcg.hpp"

class my_matrix {
    std::vector<std::vector<bool> >m;
    public:
    my_matrix(unsigned int x, unsigned int y) {
        m.resize(x, std::vector<bool>(y,false));
    }
    class matrix_row {
        std::vector<bool>& row;
    public:
        matrix_row(std::vector<bool>& r) : row(r) {
        }
        bool& operator[](unsigned int y) {
        return row.at(y);
        }
    };
    matrix_row& operator[](unsigned int x) {
        return matrix_row(m.at(x));
  }
};