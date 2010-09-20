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
        // operator definitions
        enum operator_type {
            ADD_CONTENT,        // +-
            ADD_POINTER,        // <>
            OUTPUT_CONTENT,     // .
            INPUT_CONTENT,      // ,
            LOOP_START,         // [
            LOOP_END            // ]
        };

        std::ostream&
        operator<<(std::ostream& out, const operator_type& op) {
            switch (op) {
                case ADD_CONTENT:
                    out << "ADD_CONTENT"; break;
                case ADD_POINTER:
                    out << "ADD_POINTER"; break;
                case OUTPUT_CONTENT:
                    out << "OUTPUT_CONTENT"; break;
                case INPUT_CONTENT:
                    out << "INPUT_CONTENT"; break;
                case LOOP_START:
                    out << "LOOP_START"; break;
                case LOOP_END:
                    out << "LOOP_END"; break;
            }
            return out;
        }

        // operand definition
        typedef int operand_type;

        // instruction definition
        struct instruction_type {
            operator_type op;
            operand_type operand;
        };

        std::ostream&
        operator<<(std::ostream& out, const instruction_type& i) {
            return out << i.op << "\t" << i.operand << "\n";
        }

        // array of instructions definition
        typedef std::vector<instruction_type> instructions_type;

        std::ostream&
        operator<<(std::ostream& out, const instructions_type& i) {
            std::copy(
                    i.begin(), i.end(),
                    std::ostream_iterator<instruction_type>(out));
            return out;
        }

        // class parser definitions
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
                    char c;
                    operand_type p = 0;
                    operand_type n = 0;
                    while (in.good()) {
                        in.get(c);
                        switch (c) {
                            case '>':
                                if (n != 0) { generate_instruction(ADD_CONTENT, n); n = 0; }
                                ++p;
                                break;
                            case '<':
                                if (n != 0) { generate_instruction(ADD_CONTENT, n); n = 0; }
                                --p;
                                break;
                            case '+':
                                if (p != 0) { generate_instruction(ADD_POINTER, p); p = 0; }
                                ++n;
                                break;
                            case '-':
                                if (p != 0) { generate_instruction(ADD_POINTER, p); p = 0; }
                                --n;
                                break;
                            case '.':
                                if (n != 0) { generate_instruction(ADD_CONTENT, n); n = 0; }
                                if (p != 0) { generate_instruction(ADD_POINTER, p); p = 0; }
                                generate_instruction(OUTPUT_CONTENT, 0);
                                break;
                            case ',':
                                if (n != 0) { generate_instruction(ADD_CONTENT, n); n = 0; }
                                if (p != 0) { generate_instruction(ADD_POINTER, p); p = 0; }
                                generate_instruction(INPUT_CONTENT, 0);
                                break;
                            case '[':
                                if (n != 0) { generate_instruction(ADD_CONTENT, n); n = 0; }
                                if (p != 0) { generate_instruction(ADD_POINTER, p); p = 0; }
                                generate_instruction(LOOP_START, 0);
                                break;
                            case ']':
                                if (n != 0) { generate_instruction(ADD_CONTENT, n); n = 0; }
                                if (p != 0) { generate_instruction(ADD_POINTER, p); p = 0; }
                                generate_instruction(LOOP_END, 0);
                                break;
                        }
                    }
                    pv_state = OK;
                    return true;
                }

                const instructions_type& instructions(void) const {
                    return pv_instructions;
                }

            protected:
                void generate_instruction(operator_type op, operand_type operand) {
                    instruction_type inst = {op, operand};
                    pv_instructions.push_back(inst);
                }
        };

        // class executer definitions
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
                        switch (pc->op) {
                            case ADD_CONTENT:
                                *ptr += static_cast<char>(pc->operand);
                                break;
                            case ADD_POINTER:
                                ptr += pc->operand;
                                break;
                            case OUTPUT_CONTENT:
                                out->put(*ptr);
                                break;
                            case INPUT_CONTENT:
                                in->get(*ptr);
                                break;
                            case LOOP_START:
                                if (*ptr == 0) {
                                    unsigned int nest = 1;
                                    while (nest != 0) {
                                        switch ((++pc)->op) {
                                            case LOOP_START:    ++nest; break;
                                            case LOOP_END:      --nest; break;
                                            default:            break;
                                        }
                                    }
                                }
                                break;
                            case LOOP_END:
                                if (*ptr != 0) {
                                    unsigned int nest = 1;
                                    while (nest != 0) {
                                        switch ((--pc)->op) {
                                            case LOOP_START:    --nest; break;
                                            case LOOP_END:      ++nest; break;
                                            default:            break;
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

#endif // BRAINF_CK_HPP

