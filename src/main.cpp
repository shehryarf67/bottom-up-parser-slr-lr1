#include "lr1_parser.h"
#include "slr_parser.h"
#include "tree.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace {
    void writeTextFile(const fs::path& file, const std::string& content) {
        std::ofstream out(file);
        out << content;
    }

    double totalParseTime(const std::vector<std::pair<std::string, ParseResult>>& results) {
        double sum = 0.0;
        for (const auto& entry : results) sum += entry.second.parseTimeMs;
        return sum;
    }

    std::string serializeTraceReport(const std::string& parserName, const std::vector<std::pair<std::string, ParseResult>>& results) {
        std::ostringstream out;
        out << parserName << " Parsing Trace Report\n";
        out << "========================================\n\n";
        for (std::size_t i = 0; i < results.size(); ++i) {
            out << "Input #" << (i + 1) << ": " << results[i].first << '\n';
            out << "Result: " << (results[i].second.accepted ? "Accepted" : "Rejected") << '\n';
            out << "Message: " << results[i].second.message << '\n';
            out << "Parse Time (ms): " << results[i].second.parseTimeMs << '\n';
            out << "Trace:\n";
            for (const std::string& line : results[i].second.traceLines) out << line << '\n';
            out << '\n';
        }
        return out.str();
    }

    std::string serializeParseTrees(const std::string& parserName, const std::vector<std::pair<std::string, ParseResult>>& results) {
        std::ostringstream out;
        out << parserName << " Parse Trees\n";
        out << "========================================\n\n";
        bool anyAccepted = false;
        for (std::size_t i = 0; i < results.size(); ++i) {
            const auto& input = results[i].first;
            const ParseResult& result = results[i].second;
            if (!result.accepted || !result.root) continue;
            anyAccepted = true;
            out << "Input #" << (i + 1) << ": " << input << '\n';
            out << "Reductions:\n";
            for (const std::string& r : result.reductions) out << "  - " << r << '\n';
            out << "Parse Tree:\n" << ParseTree::render(result.root) << '\n';
        }
        if (!anyAccepted) out << "No accepted strings were available for parse tree generation.\n\n";
        return out.str();
    }

    std::string serializeComparison(const ParserMetrics& slrM, const ParserMetrics& lr1M, const ParsingTable& slrT, const ParsingTable& lr1T, const std::vector<std::pair<std::string, ParseResult>>& slrR, const std::vector<std::pair<std::string, ParseResult>>& lr1R) {
        std::ostringstream out;
        out << "SLR(1) vs LR(1) Comparison\n";
        out << "========================================\n\n";
        out << "State Count Comparison\n";
        out << "  SLR(1): " << slrM.stateCount << '\n';
        out << "  LR(1):  " << lr1M.stateCount << "\n\n";
        out << "Parsing Table Density\n";
        out << "  SLR(1): ACTION cells = " << slrM.filledActionCells << ", GOTO cells = " << slrM.filledGotoCells << ", serialized bytes = " << slrM.estimatedTableBytes << '\n';
        out << "  LR(1):  ACTION cells = " << lr1M.filledActionCells << ", GOTO cells = " << lr1M.filledGotoCells << ", serialized bytes = " << lr1M.estimatedTableBytes << "\n\n";
        out << "Construction Time\n";
        out << "  SLR(1): " << slrM.buildTimeMs << " ms\n";
        out << "  LR(1):  " << lr1M.buildTimeMs << " ms\n\n";
        out << "Parsing Speed on Provided Inputs\n";
        out << "  SLR(1): " << totalParseTime(slrR) << " ms total\n";
        out << "  LR(1):  " << totalParseTime(lr1R) << " ms total\n\n";
        out << "Conflict Analysis\n";
        out << "  SLR(1) conflict count: " << slrM.conflictCount << '\n';
        if (slrT.getConflicts().empty()) out << "  SLR(1): No conflicts detected.\n";
        else for (const Conflict& c : slrT.getConflicts()) out << "  SLR(1): State I" << c.state << ", Symbol '" << c.symbol << "' -> " << c.type << '\n';
        out << "  LR(1) conflict count: " << lr1M.conflictCount << '\n';
        if (lr1T.getConflicts().empty()) out << "  LR(1): No conflicts detected.\n";
        else for (const Conflict& c : lr1T.getConflicts()) out << "  LR(1): State I" << c.state << ", Symbol '" << c.symbol << "' -> " << c.type << '\n';
        out << '\n';
        if (slrM.conflictCount > 0 && lr1M.conflictCount == 0) out << "Observation: LR(1) resolves the conflicts encountered by SLR(1) for this grammar.\n";
        else if (slrM.conflictCount == 0 && lr1M.conflictCount == 0) out << "Observation: Both parsers handle this grammar without table conflicts.\n";
        else out << "Observation: Review the listed conflicts.\n";
        return out.str();
    }

    void ensureOutputPlaceholders(const fs::path& outputDir) {
        const std::vector<std::string> files = {"augmented_grammar.txt","slr_items.txt","slr_parsing_table.txt","slr_trace.txt","lr1_items.txt","lr1_parsing_table.txt","lr1_trace.txt","comparison.txt","parse_trees.txt"};
        for (const std::string& name : files) {
            fs::path target = outputDir / name;
            if (!fs::exists(target)) writeTextFile(target, "Generated after running the program.\n");
        }
    }
}

int main(int argc, char* argv[]) {
    std::string mode = "all";
    std::string grammarPath;
    std::string inputPath;
    std::string outputDir = "output";

    if (argc == 4 || argc == 5) {
        mode = argv[1];
        grammarPath = argv[2];
        inputPath = argv[3];
        if (argc == 5) outputDir = argv[4];
    } else if (argc != 1) {
        std::cerr << "Usage:\n  ./parser all <grammar-file> <input-file> [output-dir]\n  ./parser slr <grammar-file> <input-file> [output-dir]\n  ./parser lr1 <grammar-file> <input-file> [output-dir]\n  ./parser compare <grammar-file> <input-file> [output-dir]\n";
        return 1;
    }

    fs::create_directories(outputDir);
    if (argc == 1) {
        ensureOutputPlaceholders(outputDir);
        std::cout << "Created output placeholders in '" << outputDir << "'.\n";
        return 0;
    }

    bool runSLR = (mode == "all" || mode == "slr" || mode == "compare");
    bool runLR1 = (mode == "all" || mode == "lr1" || mode == "compare");
    if (!runSLR && !runLR1) {
        std::cerr << "Unknown mode: " << mode << '\n';
        return 1;
    }

    SLRParser slr;
    LR1Parser lr1;
    std::vector<std::pair<std::string, ParseResult>> slrResults;
    std::vector<std::pair<std::string, ParseResult>> lr1Results;

    if (runSLR) {
        std::string error;
        if (!slr.initialize(grammarPath, error)) { std::cerr << error << '\n'; return 1; }
        slr.build();
        slrResults = slr.parseFile(inputPath);
        writeTextFile(fs::path(outputDir) / "augmented_grammar.txt", slr.serializeGrammarSummary());
        writeTextFile(fs::path(outputDir) / "slr_items.txt", slr.serializeItems());
        writeTextFile(fs::path(outputDir) / "slr_parsing_table.txt", slr.serializeTable());
        writeTextFile(fs::path(outputDir) / "slr_trace.txt", serializeTraceReport("SLR(1)", slrResults));
    }
    if (runLR1) {
        std::string error;
        if (!lr1.initialize(grammarPath, error)) { std::cerr << error << '\n'; return 1; }
        lr1.build();
        lr1Results = lr1.parseFile(inputPath);
        if (!runSLR) writeTextFile(fs::path(outputDir) / "augmented_grammar.txt", lr1.serializeGrammarSummary());
        writeTextFile(fs::path(outputDir) / "lr1_items.txt", lr1.serializeItems());
        writeTextFile(fs::path(outputDir) / "lr1_parsing_table.txt", lr1.serializeTable());
        writeTextFile(fs::path(outputDir) / "lr1_trace.txt", serializeTraceReport("LR(1)", lr1Results));
    }

    std::ostringstream parseTrees;
    if (runSLR) parseTrees << serializeParseTrees("SLR(1)", slrResults);
    if (runLR1) parseTrees << serializeParseTrees("LR(1)", lr1Results);
    writeTextFile(fs::path(outputDir) / "parse_trees.txt", parseTrees.str());

    if (runSLR && runLR1) writeTextFile(fs::path(outputDir) / "comparison.txt", serializeComparison(slr.getMetrics(), lr1.getMetrics(), slr.getTable(), lr1.getTable(), slrResults, lr1Results));
    else writeTextFile(fs::path(outputDir) / "comparison.txt", "Run the program in 'all' or 'compare' mode to generate a full comparison report.\n");

    std::cout << "Parsing artifacts generated successfully in: " << outputDir << '\n';
    return 0;
}
