/***************************************************************************************
SLIME -- Copyright (c) 2022, Oscar Riveros, oscar.riveros@gmail.com,
Santiago, Chile. https://github.com/maxtuno/SLIME

SLIME -- Copyright (c) 2021, Oscar Riveros, oscar.riveros@gmail.com,
Santiago, Chile. https://github.com/maxtuno/SLIME

DurianSat -- Copyright (c) 2020, Arijit Shaw, Kuldeep S. Meel

SLIME -- Copyright (c) 2019, Oscar Riveros, oscar.riveros@gmail.com,
Santiago, Chile. https://maxtuno.github.io/slime-sat-solver

Maple_LCM_Dist_Chrono -- Copyright (c) 2018, Vadim Ryvchin, Alexander Nadel

GlucoseNbSAT -- Copyright (c) 2016,Chu Min LI,Mao Luo and Fan Xiao
                           Huazhong University of science and technology, China
                           MIS, Univ. Picardie Jules Verne, France

MapleSAT -- Copyright (c) 2016, Jia Hui Liang, Vijay Ganesh

MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
           Copyright (c) 2007-2010  Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
\"Software\"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <break_id/BreakID.hh>
#include <Dimacs.h>
#include <SimpSolver.h>
#include <SolverTypes.h>
#include <iostream>

#ifdef MASSIVE

#include <mpi.h>

#endif

#define DRAT // Generate unsat proof.

using namespace SLIME;

#if _WIN32 || _WIN64

#include <io.h>
#include <fcntl.h>

void printHeader() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wcout << L"c                                         \n";
    std::wcout << L"c   ██████  ██▓     ██▓ ███▄ ▄███▓▓█████  \n";
    std::wcout << L"c ▒██    ▒ ▓██▒    ▓██▒▓██▒▀█▀ ██▒▓█   ▀  \n";
    std::wcout << L"c ░ ▓██▄   ▒██░    ▒██▒▓██    ▓██░▒███    \n";
    std::wcout << L"c   ▒   ██▒▒██░    ░██░▒██    ▒██ ▒▓█  ▄  \n";
    std::wcout << L"c ▒██████▒▒░██████▒░██░▒██▒   ░██▒░▒████▒ \n";
    std::wcout << L"c ▒ ▒▓▒ ▒ ░░ ▒░▓  ░░▓  ░ ▒░   ░  ░░░ ▒░ ░ \n";
    std::wcout << L"c ░ ░▒  ░ ░░ ░ ▒  ░ ▒ ░░  ░      ░ ░ ░  ░ \n";
    std::wcout << L"c ░  ░  ░    ░ ░    ▒ ░░      ░      ░    \n";
    std::wcout << L"c       ░      ░  ░ ░         ░      ░  ░ \n";
    std::wcout << L"c                                         \n";
    std::wcout << L"c              Oscar Riveros              \n";
    std::wcout << L"c                                         \n";
    std::wcout << L"c             [DEEP-LEARNING]             \n";
    std::wcout << L"c                                         \n";
    _setmode(_fileno(stdout), _O_TEXT);
}

#else

void printHeader() {
    printf("c                                         \n");
    printf("c   ██████  ██▓     ██▓ ███▄ ▄███▓▓█████  \n");
    printf("c ▒██    ▒ ▓██▒    ▓██▒▓██▒▀█▀ ██▒▓█   ▀  \n");
    printf("c ░ ▓██▄   ▒██░    ▒██▒▓██    ▓██░▒███    \n");
    printf("c   ▒   ██▒▒██░    ░██░▒██    ▒██ ▒▓█  ▄  \n");
    printf("c ▒██████▒▒░██████▒░██░▒██▒   ░██▒░▒████▒ \n");
    printf("c ▒ ▒▓▒ ▒ ░░ ▒░▓  ░░▓  ░ ▒░   ░  ░░░ ▒░ ░ \n");
    printf("c ░ ░▒  ░ ░░ ░ ▒  ░ ▒ ░░  ░      ░ ░ ░  ░ \n");
    printf("c ░  ░  ░    ░ ░    ▒ ░░      ░      ░    \n");
    printf("c       ░      ░  ░ ░         ░      ░  ░ \n");
    printf("c                                         \n");
    printf("c              Oscar Riveros              \n");
    printf("c                                         \n");
    printf("c             [DEEP-LEARNING]             \n");
    printf("c                                         \n");
}

#endif

int main(int argc, char *argv[]) {

    if (argc == 1) {
        printf("SLIME -- Copyright (c) 2022, Oscar Riveros, oscar.riveros@gmail.com,\n");
        printf("Santiago, Chile. https://github.com/maxtuno/SLIME\n");
        printf("\n");
        printf("SLIME -- Copyright (c) 2021, Oscar Riveros, oscar.riveros@gmail.com,\n");
        printf("Santiago, Chile. https://github.com/maxtuno/SLIME\n");
        printf("\n");
        printf("DurianSat -- Copyright (c) 2020, Arijit Shaw, Kuldeep S. Meel\n");
        printf("\n");
        printf("SLIME -- Copyright (c) 2019, Oscar Riveros, oscar.riveros@gmail.com,\n");
        printf("Santiago, Chile. https://maxtuno.github.io/slime-sat-solver\n");
        printf("\n");
        printf("Maple_LCM_Dist_Chrono -- Copyright (c) 2018, Vadim Ryvchin, Alexander Nadel\n");
        printf("\n");
        printf("GlucoseNbSAT -- Copyright (c) 2016,Chu Min LI,Mao Luo and Fan Xiao\n");
        printf("                Huazhong University of science and technology, China\n");
        printf("                MIS, Univ. Picardie Jules Verne, France\n");
        printf("\n");
        printf("MapleSAT -- Copyright (c) 2016, Jia Hui Liang, Vijay Ganesh\n");
        printf("\n");
        printf("MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson\n");
        printf("           Copyright (c) 2007-2010  Niklas Sorensson\n");
        printf("\n");
        printf("Permission is hereby granted, free of charge, to any person obtaining a\n");
        printf("copy of this software and associated documentation files (the\n");
        printf("\"Software\", to deal in the Software without restriction, including\n");
        printf("without limitation the rights to use, copy, modify, merge, publish,\n");
        printf("distribute, sublicense, and/or sell copies of the Software, and to\n");
        printf("permit persons to whom the Software is furnished to do so, subject to\n");
        printf("the following conditions:\n");
        printf("\n");
        printf("The above copyright notice and this permission notice shall be included\n");
        printf("in all copies or substantial portions of the Software.\n");
        printf("\n");
        printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n");
        printf("OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n");
        printf("MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n");
        printf("NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE\n");
        printf("LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION\n");
        printf("OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION\n");
        printf("WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n");
        printf("\n");
        printf("For the files \"saucy.hpp\" and \"saucy.cpp\" the following license holds:\n");
        printf("\n");
        printf("COPYRIGHT (C) 2008\n");
        printf("THE REGENTS OF THE UNIVERSITY OF MICHIGAN\n");
        printf("ALL RIGHTS RESERVED\n");
        printf("\n");
        printf("PERMISSION IS GRANTED TO USE, COPY, CREATE DERIVATIVE WORKS\n");
        printf("AND REDISTRIBUTE THIS SOFTWARE AND SUCH DERIVATIVE WORKS FOR\n");
        printf("NON-COMMERCIAL PURPOSES, SO LONG AS THE COPYRIGHT NOTICE ABOVE,\n");
        printf("THIS GRANT OF PERMISSION, AND THE DISCLAIMER BELOW APPEAR IN ALL\n");
        printf("COPIES MADE; AND SO LONG AS THE NAME OF THE UNIVERSITY OF MICHIGAN\n");
        printf("IS NOT USED IN ANY ADVERTISING OR PUBLICITY PERTAINING TO THE USE\n");
        printf("OR DISTRIBUTION OF THIS SOFTWARE WITHOUT SPECIFIC, WRITTEN PRIOR\n");
        printf("AUTHORIZATION.\n");
        printf("\n");
        printf("THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE\n");
        printf("UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND WITHOUT\n");
        printf("WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER EXPRESS\n");
        printf("OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF\n");
        printf("MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE REGENTS\n");
        printf("OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE LIABLE FOR ANY DAMAGES,\n");
        printf("INCLUDING SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,\n");
        printf("WITH RESPECT TO ANY CLAIM ARISING OUT OF OR IN CONNECTION WITH THE\n");
        printf("USE OF THE SOFTWARE, EVEN IF IT HAS BEEN OR IS HEREAFTER ADVISED OF\n");
        printf("THE POSSIBILITY OF SUCH DAMAGES.\n");
        printf("\n");
        printf("For all other source code files (on break_id folder), the following license holds:\n");
        printf("\n");
        printf("Copyright 2010-2019 KU Leuven, Departement Computerwetenschappen,\n");
        printf("Celestijnenlaan 200A, B-3001 Leuven, Belgium\n");
        printf("\n");
        printf("Permission is hereby granted, free of charge, to any person obtaining a copy\n");
        printf("of this software and associated documentation files (the \"Software\"), to deal\n");
        printf("in the Software without restriction, including without limitation the rights\n");
        printf("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n");
        printf("copies of the Software, and to permit persons to whom the Software is\n");
        printf("furnished to do so, subject to the following conditions:\n");
        printf("\n");
        printf("The above copyright notice and this permission notice shall be included in all\n");
        printf("copies or substantial portions of the Software.\n");
        printf("\n");
        printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n");
        printf("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n");
        printf("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n");
        printf("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n");
        printf("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n");
        printf("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n");
        printf("\n");
        printf("For Standalone Multiprecision from Boost-1.80\n");
        printf("\n");
        printf("Boost Software License - Version 1.0 - August 17th, 2003\n");
        printf("\n");
        printf("Permission is hereby granted, free of charge, to any person or organization\n");
        printf("obtaining a copy of the software and accompanying documentation covered by\n");
        printf("this license (the \"Software\") to use, reproduce, display, distribute,\n");
        printf("execute, and transmit the Software, and to prepare derivative works of the\n");
        printf("Software, and to permit third-parties to whom the Software is furnished to\n");
        printf("do so, all subject to the following:\n");
        printf("\n");
        printf("The copyright notices in the Software and this entire statement, including\n");
        printf("the above license grant, this restriction and the following disclaimer,\n");
        printf("must be included in all copies of the Software, in whole or in part, and\n");
        printf("all derivative works of the Software, unless such copies or derivative\n");
        printf("works are solely in the form of machine-executable object code generated by\n");
        printf("a source language processor.\n");
        printf("\n");
        printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n");
        printf("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n");
        printf("FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT\n");
        printf("SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE\n");
        printf("FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,\n");
        printf("ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n");
        printf("DEALINGS IN THE SOFTWARE.\n");
        printf("\n");
        printf("For DeZeroCpp\n");
        printf("\n");
        printf("MIT License\n");
        printf("\n");
        printf("Copyright (c) 2020 bugphobia\n");
        printf("\n");
        printf("Permission is hereby granted, free of charge, to any person obtaining a copy\n");
        printf("of this software and associated documentation files (the \"Software\"), to deal\n");
        printf("in the Software without restriction, including without limitation the rights\n");
        printf("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n");
        printf("copies of the Software, and to permit persons to whom the Software is\n");
        printf("furnished to do so, subject to the following conditions:\n");
        printf("\n");
        printf("The above copyright notice and this permission notice shall be included in all\n");
        printf("copies or substantial portions of the Software.\n");
        printf("\n");
        printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n");
        printf("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n");
        printf("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n");
        printf("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n");
        printf("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n");
        printf("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n");
        printf("SOFTWARE.\n");
        printf("\n");
        printf("For NumCpp\n");
        printf("\n");
        printf("MIT License\n");
        printf("\n");
        printf("Copyright (c) 2020 David Pilger\n");
        printf("\n");
        printf("Permission is hereby granted, free of charge, to any person obtaining a copy\n");
        printf("of this software and associated documentation files (the \"Software\"), to deal\n");
        printf("in the Software without restriction, including without limitation the rights\n");
        printf("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n");
        printf("copies of the Software, and to permit persons to whom the Software is\n");
        printf("furnished to do so, subject to the following conditions:\n");
        printf("\n");
        printf("The above copyright notice and this permission notice shall be included in all\n");
        printf("copies or substantial portions of the Software.\n");
        printf("\n");
        printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n");
        printf("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n");
        printf("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n");
        printf("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n");
        printf("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n");
        printf("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n");
        printf("SOFTWARE.\n");
        exit(0);
    }

    parseOptions(argc, argv, true);

    SimpSolver S;

    char *drup_file = const_cast<char*>("");

#ifdef MASSIVE
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &S.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &S.size);
    if (S.rank % 4 == 0) {
        S.use_distance = false;
        S.invert_polarity = false;
    } else if (S.rank % 4 == 1) {
        S.use_distance = true;
        S.invert_polarity = false;
    }
    else if (S.rank % 4 == 2) {
        S.use_distance = false;
        S.invert_polarity = true;
    } else {
        S.use_distance = true;
        S.invert_polarity = true;
    }
2;
#else
    S.rank = 0;
    S.size = 1;
#endif

    S.render = true;

    if (S.rank == 0) {
        printHeader();
    }

#ifdef LOG
    S.log = true;
#else
    S.log = false;
#endif

#ifdef DRAT
    if (argc > 2) {
#ifdef MASSIVE
        char *proof_file = (char *) calloc(strlen(argv[2]), sizeof(char) + 10);
        sprintf(proof_file, "%s_%d", argv[2], S.rank);
        S.drup_file = fopen(proof_file, "wb");
#else
        drup_file = argv[2];
        S.drup_file = fopen(drup_file, "wb+");
#endif
    }
#endif

    break_id_to_slime(argv[1], drup_file, S, S.rank);

    FILE *in = fopen(argv[1], "rb");
    if (in == NULL) {
        std::cout << "c ERROR! Could not open file: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }
    int nVars = get_DIMACS_variables(in);
    fclose(in);

    S.eliminate();

    vec<Lit> assumptions;
    lbool result = S.solveLimited(assumptions);

#ifdef MASSIVE
    printf("c [%i]\n", S.rank);
#endif

    printf(result == l_True ? "s SATISFIABLE\nv " : result == l_False ? "s UNSATISFIABLE\n" : "s UNKNOWN\n");
    if (result == l_True) {
        for (int i = 0; i < nVars; i++) {
            if (S.model[i] != l_Undef) {
                printf("%s%s%d", (i == 0) ? "" : " ", (S.model[i] == l_True) ? "" : "-", i + 1);
            }
            // if (i && i % 10 == 0) {
            //     printf("\nv");
            // }
        }
        printf(" 0\n");
        fflush(stdout);
    } else {
#ifdef DRAT
        if (argc > 2) {
            fputc('a', S.drup_file);
            fputc(0, S.drup_file);
            fclose(S.drup_file);
        }
#endif
    }

    if (argc > 3) {
#ifdef MASSIVE
        char *model_file = (char *) calloc(strlen(argv[3]), sizeof(char) + 10);
        sprintf(model_file, "%s_%d", argv[3], S.rank);
        FILE *model = fopen(model_file, "wb");
#else
        FILE *model = fopen(argv[3], "w");
#endif
        fprintf(model, result == l_True ? "SAT\n" : result == l_False ? "UNSAT\n" : "UNKNOWN\n");
        if (result == l_True) {
            for (int i = 0; i < nVars; i++)
                if (S.model[i] != l_Undef) {
                    fprintf(model, "%s%s%d", (i == 0) ? "" : " ", (S.model[i] == l_True) ? "" : "-", i + 1);
                }
            fprintf(model, " 0\n");
        }
        fclose(model);
    }

#ifdef MASSIVE
    MPI_Abort(MPI_COMM_WORLD, EXIT_SUCCESS);
    MPI_Finalize();
#endif

    exit(result == l_True ? 10 : result == l_False ? 20 : 0);
}
