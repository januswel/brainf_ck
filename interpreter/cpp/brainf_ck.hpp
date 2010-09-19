/*
 * brainf_ck.hpp
 *  brainf*ck interpreter
 *
 *  - lang::brainf_ck::parser
 *      A class to parse brainf*ck source codes and build intermediate codes
 *  - lang::brainf_ck::executer
 *      A class to execute the intermediate codes that are built by the class
 *      lang::brainf_ck::parser
 *
 *  written by janus_wel<janus.wel.3@gmail.com>
 *  This source code is in public domain, and has NO WARRANTY.
 * */

#ifndef BRAINF_CK_HPP
#define BRAINF_CK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

namespace lang {
    namespace brainf_ck {
        typedef std::vector<char> instructions_type;

        class parser {
            public:
                enum state_type {
                    OK,
                    PARSE_FAILED
                };

            private:
                instructions_type pv_instructions;

                state_type pv_state;
                std::string pv_errmsg;

            public:
                parser(void) : pv_state(OK) {}
                parser(std::istream& in) : pv_state(OK) {
                    try {
                        if (!parse(in)) {
                            pv_state = PARSE_FAILED;
                        }
                    }
                    catch (std::exception& ex) {
                        pv_state = PARSE_FAILED;
                        pv_errmsg = ex.what();
                    }
                }

                operator bool(void) const {
                    return (pv_state == OK);
                }
                const std::string& errmsg(void) const {
                    return pv_errmsg;
                }

                bool parse(std::istream& in) {
                    std::copy(
                            std::istream_iterator<char>(in),
                            std::istream_iterator<char>(),
                            std::back_inserter(pv_instructions));
                    pv_state = OK;
                    return true;
                }

                const instructions_type& instructions(void) const {
                    return pv_instructions;
                }
        };

        class executer {
            public:
                typedef instructions_type::const_iterator programcounter_type;

                static const unsigned int MEMORY_SIZE = 30000;

            private:
                std::istream* in;
                std::ostream* out;

            public:
                executer(void) : in(&(std::cin)), out(&(std::cout)) {}
                executer(std::istream& in, std::ostream& out)
                    : in(&in), out(&out) {}

                void execute(const instructions_type& insts) {
                    std::vector<char> memory(MEMORY_SIZE);
                    char* ptr = &memory[0];

                    for (   programcounter_type pc = insts.begin();
                            pc != insts.end();
                            ++pc) {
                        switch (*pc) {
                            case '>':   ++ptr; break;
                            case '<':   --ptr; break;
                            case '+':   ++(*ptr); break;
                            case '-':   --(*ptr); break;
                            case '.':   out->put(*ptr); break;
                            case ',':   in->get(*ptr); break;
                            case '[':   if (*ptr == 0) {
                                            unsigned int nest = 1;
                                            while (nest != 0) {
                                                switch (*++pc) {
                                                    case '[':   ++nest; break;
                                                    case ']':   --nest; break;
                                                }
                                            }
                                        }
                                        break;
                            case ']':   if (*ptr != 0) {
                                            unsigned int nest = 1;
                                            while (nest != 0) {
                                                switch (*--pc) {
                                                    case '[':   --nest; break;
                                                    case ']':   ++nest; break;
                                                }
                                            }
                                        }
                                        break;
                        }
                    }
                }
        };
    }
}

std::ostream&
operator<<(std::ostream& out, const lang::brainf_ck::instructions_type& i) {
    std::copy(i.begin(), i.end(), std::ostream_iterator<char>(out));
    return out;
}

#endif // BRAINF_CK_HPP

