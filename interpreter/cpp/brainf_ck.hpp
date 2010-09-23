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

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stack>
#include <string>
#include <vector>
#include <stdexcept>

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

#define GEN_INST(OPERATOR, OPERAND)                     \
        if (OPERAND != 0) {                             \
            generate_instruction(OPERATOR, OPERAND);    \
            OPERAND = 0;                                \
        }

        // class parser definitions
        class parser {
            public:
                enum state_type {
                    OK,
                    PARSE_FAILED
                };

                typedef instructions_type::size_type position_type;
                typedef int distance_type;

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
                    std::stack<position_type> loop_stack;
                    while (in.good()) {
                        in.get(c);
                        switch (c) {
                            case '>':
                                GEN_INST(ADD_CONTENT, n);
                                ++p;
                                break;
                            case '<':
                                GEN_INST(ADD_CONTENT, n);
                                --p;
                                break;
                            case '+':
                                GEN_INST(ADD_POINTER, p);
                                ++n;
                                break;
                            case '-':
                                GEN_INST(ADD_POINTER, p);
                                --n;
                                break;
                            case '.':
                                GEN_INST(ADD_CONTENT, n);
                                GEN_INST(ADD_POINTER, p);
                                generate_instruction(OUTPUT_CONTENT, 0);
                                break;
                            case ',':
                                GEN_INST(ADD_CONTENT, n);
                                GEN_INST(ADD_POINTER, p);
                                generate_instruction(INPUT_CONTENT, 0);
                                break;
                            case '[':
                                GEN_INST(ADD_CONTENT, n);
                                GEN_INST(ADD_POINTER, p);
                                // Save the position of '['.
                                loop_stack.push(pv_instructions.size());
                                // An operand 0 is temporary. It is rewritten
                                // by processing corresponding ']' later.
                                generate_instruction(LOOP_START, 0);
                                break;
                            case ']':
                                GEN_INST(ADD_CONTENT, n);
                                GEN_INST(ADD_POINTER, p);

                                if (loop_stack.size() == 0) {
                                    throw std::runtime_error("Found extra ]");
                                }

                                // Get the position of corresponding '['.
                                position_type loop_start = loop_stack.top();
                                loop_stack.pop();
                                // Calculate a distance of '[' and ']'.
                                distance_type distance = pv_instructions.size() - loop_start;
                                // Rewrite the operand of '['.
                                pv_instructions[loop_start].operand = distance;
                                // A value of operand is the distance from ']'
                                // to the precedent of corresponding '['.
                                generate_instruction(LOOP_END, -1 - distance);
                                break;
                        }
                    }

                    GEN_INST(ADD_CONTENT, n);
                    GEN_INST(ADD_POINTER, p);

                    if (loop_stack.size() != 0) {
                        throw std::runtime_error("Found extra [");
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
                    const char* const start = ptr;
                    const char* const end = &memory[MEMORY_SIZE - 1];

                    for (   programcounter_type pc = insts.begin();
                            pc != insts.end();
                            ++pc) {
                        switch (pc->op) {
                            case ADD_CONTENT:
                                *ptr += static_cast<char>(pc->operand);
                                break;
                            case ADD_POINTER:
                                ptr += pc->operand;
                                if (ptr < start || end < ptr) {
                                    throw std::runtime_error("memory access violation");
                                }
                                break;
                            case OUTPUT_CONTENT:
                                out->put(*ptr);
                                break;
                            case INPUT_CONTENT:
                                in->get(*ptr);
                                break;
                            case LOOP_START:
                                if (*ptr == 0) {
                                    std::advance(pc, pc->operand);
                                }
                                break;
                            case LOOP_END:
                                if (*ptr != 0) {
                                    std::advance(pc, pc->operand);
                                }
                                break;
                        }
                    }
                }
        };
    }
}

#endif // BRAINF_CK_HPP

