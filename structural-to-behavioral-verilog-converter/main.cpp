#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include "sstream"
#include "vector"

#define INPUT "input"
#define OUTPUT "output"
#define WIRE "wire"
#define NOT "not"
#define OR "or"
#define AND "and"
#define AND_OPERATOR '&'
#define OR_OPERATOR '|'

#define TESTBENCH_ITERATIONS 20
#define TESTBENCH_START_DELAY 10
#define TESTBENCH_ITERATION_DELAY 15

using namespace std;

template<class T>
void printVector(const vector<T> &vec, const string &title = "vector") {
    cout << "*********\n";
    cout << title << ":" << endl;
    int i = 0;
    for (const auto &x: vec) {
        cout << "[" << i++ << "]: " << x << endl;
    }
}

void delStartingSpaces(string &str) {
    int i = 0;
    for (char c: str) {
        if (!isspace(c)) break;
        i++;
    }
    str.erase(0, i);
}

void removeCharFromString(string &item, const char &c) {
    item.erase(remove(item.begin(), item.end(), c), item.end());
}

void removeNonAlphaNumericChars(string &s) {
    s.erase(std::remove_if(
            s.begin(),
            s.end(),
            [](auto const &c) -> bool { return !std::isalnum(c); }), s.end()
    );
}

void stripNewLineCharFromString(string &s) {
    s.erase(std::remove_if(
            s.begin(),
            s.end(),
            [](auto const &c) -> bool { return c == '\n' || c == '\r'; }), s.end()
    );
}

vector<string> splitBy(const string &code, const char &c) {
    stringstream ss(code);
    string item;
    vector<string> result;

    while (getline(ss, item, c)) {
        if (item.empty()) continue;
        removeCharFromString(item, ';');
        removeCharFromString(item, '\n');
        removeCharFromString(item, '\r');
        result.push_back(item);
    }
    return result;
}

vector<string> getFileByLines(const string &fileName) {
    vector<string> lines;
    ifstream file(fileName);

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            delStartingSpaces(line);
            if (!line.empty())
                lines.push_back(line);
        }
        file.close();
    } else {
        cerr << "input file: " << fileName << " not found!";
    }

    return lines;
}

string getStringWithoutChar(const string &str, const char &c) {
    string res;
    for (auto chr: str) {
        if (chr != c)
            res.push_back(chr);
    }
    return res;
}

std::pair<string, string> resolveUnaryOperator(vector<string> words, const char sign = '~') {
    string res = words[1];
    string arg = words[2];

    vector<string> splatted = splitBy(res, '(');
    auto result = getStringWithoutChar(splatted[1], ',');

    string detailedExpression = sign + arg;
    detailedExpression = getStringWithoutChar(detailedExpression, ')');

    return {result, detailedExpression}; // first: left-side variable, second: right-side expression
}

std::pair<string, string> resolveBinaryOperators(vector<string> words, const char &sign) {
    string res = words[1];
    vector<string> splatted = splitBy(res, '(');
    auto result = getStringWithoutChar(splatted[1], ',');

    string first = words[2];
    string second = words[3];
    removeCharFromString(second, ')');
    vector<string> names = splitBy(first, ',');
    first = getStringWithoutChar(names[0], ',');

    string expression = first + " " + sign + " " + second;
    expression.append(")");
    expression.insert(0, "(");

    return {result, expression}; // first: left-side variable, second: right-side expression
}

string simplify(const vector<std::pair<string, string>> &expressions, int i) {
    auto current = expressions[i]; //output
    bool shouldContinue;
    do {
        shouldContinue = false;
        for (const auto &expr: expressions) {
            size_t position = current.second.find(expr.first);
            if (position == string::npos) {
                continue;
            }
            shouldContinue = true;
            current.second.replace(position, expr.first.length(), expr.second);
        }
    } while (shouldContinue);

    return "assign " + current.first + " = " + current.second + ";";
}

void createOutput(
        const string &outputFileName,
        const vector<string> &assigns,
        const vector<string> &inputs,
        const vector<string> &outputs
) {
    ofstream file;
    file.open(outputFileName);
    file << "module Behavioral(\n";
    for (const auto &inp: inputs) {
        file << "\tinput " + inp + "," << endl;
    }
    for (int i = 0; i < outputs.size(); i++) {
        if (i == outputs.size() - 1) {
            file << "\toutput " + outputs[i] << endl;
            break;
        }
        file << "\toutput " + outputs[i] + "," << endl;
    }
    file << ");\n\n";
    for (const auto &expression: assigns) {
        file << "\t" << expression << endl;
    }

    file << endl << "endmodule" << endl;
    file.close();
}

void createTestBench(
        const string &outputFileName,
        const vector<string> &inputs,
        const vector<string> &outputs,
        const string &structuralModuleName = "Structural",
        const string &behavioralModuleName = "Behavioral"
) {
    vector<string> structural, behavioral;
    ofstream file;
    file.open(outputFileName);

    file << "module Testbench();" << endl;
    file << endl << '\t' << "// behavioral" << endl;

    for (const auto &inp: inputs) {
        string name = "b_" + inp;
        behavioral.push_back(name);
        file << '\t' << "reg " + name + ";" << endl;
    }
    for (const auto &out: outputs) {
        string name = "b_" + out;
        behavioral.push_back(name);
        file << '\t' << "wire " + name + ";" << endl;
    }

    file << endl << '\t' << "// structural" << endl;
    for (const auto &inp: inputs) {
        string name = "s_" + inp;
        structural.push_back(name);
        file << '\t' << "reg " + name + ";" << endl;
    }
    for (const auto &out: outputs) {
        string name = "s_" + out;
        structural.push_back(name);
        file << '\t' << "wire " + name + ";" << endl;
    }

    file << endl << '\t' << structuralModuleName << " S_UUT(" << endl;
    for (int i = 0; i < structural.size(); i++) {
        if (i == structural.size() - 1) {
            file << '\t' << '\t' << structural[i] << endl;
            break;
        }
        file << '\t' << '\t' << structural[i] << ',' << endl;
    }
    file << '\t' << ");" << endl;

    file << endl << '\t' << behavioralModuleName << " B_UUT(" << endl;
    for (int i = 0; i < behavioral.size(); i++) {
        if (i == behavioral.size() - 1) {
            file << '\t' << '\t' << behavioral[i] << endl;
            break;
        }
        file << '\t' << '\t' << behavioral[i] << ',' << endl;
    }

    file << '\t' << ");" << endl;
    file << endl << '\t' << "initial begin" << endl;
    file << '\t' << '\t' << "repeat(" << TESTBENCH_ITERATIONS << ") begin" << endl << "\t\t#" << TESTBENCH_START_DELAY
         << endl;
    for (int i = 0; i < inputs.size(); i++) {
        file << '\t' << '\t' << '\t' << behavioral[i] << " = " << "$random();" << endl;
        file << '\t' << '\t' << '\t' << structural[i] << " = " << behavioral[i] << ";" << endl;
    }
    file << "\t\tend" << endl << "\t\t#" << TESTBENCH_ITERATION_DELAY << " $stop;" << endl << "\tend" << endl;
    file << endl << "endmodule" << endl;
    file.close();
}

vector<string> getBehavioralExpressions(vector<std::pair<string, string>> expressions, vector<string> outputs) {
    vector<string> result;
    for (int i = 0; i < expressions.size(); i++) {
        auto expr = expressions[i];
        if (find(outputs.begin(), outputs.end(), expr.first) != outputs.end()) {
            result.push_back(simplify(expressions, i));
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    string inputFile = (argc >= 2) ? argv[1] : "input.v";
    string outputFile = (argc >= 3) ? argv[2] : "output.v";
    string testBenchFile = (argc >= 4) ? argv[3] : "test_data.v";

    vector<string> lines = getFileByLines(inputFile);
    vector<string> inputs, wires, outputs;
    vector<std::pair<string, string>> expressions;

    string structuralModuleName = getStringWithoutChar(splitBy(lines[0], ' ')[1], '(');
    stripNewLineCharFromString(structuralModuleName);

    for (const auto &line: lines) {
        vector<string> words = splitBy(line, ' ');
        string first = words[0];
        string second = (words.size() > 1) ? words[1] : words[0];
        removeNonAlphaNumericChars(second);

        if (first == INPUT) {
            inputs.push_back(second);
        } else if (first == OUTPUT) {
            outputs.push_back(second);
        } else if (first == WIRE) {
            wires.push_back(second);
        } else if (first == NOT) {
            expressions.push_back(resolveUnaryOperator(words));
        } else if (first == AND) {
            expressions.push_back(resolveBinaryOperators(words, AND_OPERATOR));
        } else if (first == OR) {
            expressions.push_back(resolveBinaryOperators(words, OR_OPERATOR));
        }
    }

    vector<string> assignments = getBehavioralExpressions(expressions, outputs);

    createOutput(outputFile, assignments, inputs, outputs);
    createTestBench(testBenchFile, inputs, outputs, structuralModuleName);

    printVector(inputs, "inputs");
    printVector(wires, "wires");
    printVector(outputs, "outputs");
    printVector(assignments, "results");

    return 0;
}
