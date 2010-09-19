/*
 * bfi.cpp
 *  brainf*ck interpreter
 *
 *  written by janus_wel<janus.wel.3@gmail.com>
 *  This source code is in public domain, and has NO WARRANTY.
 * */

#include "brainf_ck.hpp"
#include <iostream>
#include <fstream>
#include <memory>

using namespace lang::brainf_ck;

int main(const int argc, const char* const argv[]) {
    std::istream in(std::cin.rdbuf());
    std::auto_ptr<std::filebuf> fb(new std::filebuf);

    if (2 <= argc) {
        fb->open(argv[1], std::ios_base::in);
        in.rdbuf(fb.get());
    }

    in >> std::noskipws;

    parser p(in);
    if (!p) {
        std::cerr
            << "parse failed: "
            << p.errmsg()
            << std::endl;
        return 1;
    }
    std::cerr
        << p.instructions()
        << std::endl;

    executer e;
    e.execute(p.instructions());

    std::cout << std::endl;

    return 0;
}

