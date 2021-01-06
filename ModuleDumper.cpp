#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Parse/Parser.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

namespace {
struct Dumper {
  std::string Name;
  SmallVector<std::string, 32> Deps;
};

class MyPPCallbacks : public PPCallbacks {
  CompilerInstance &CI;
  Dumper &D;

  virtual void moduleImport(SourceLocation ImportLoc, ModuleIdPath Path,
                            const clang::Module *Imported) {
    for (const auto *It = Path.begin(); It != Path.end(); ++It) {
      auto Name = It->first->getName();
      D.Deps.push_back(Name.str());
      std::string Src = std::string("module ") + Name.str() + "{}";
      CI.createModuleFromSource(ImportLoc, Name, Src);
    }
  }

public:
  MyPPCallbacks(CompilerInstance &CI, Dumper &D) : CI(CI), D(D) {}
};

class MyAction : public ASTFrontendAction {
  CompilationDatabase &CD;
  Dumper D;

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    D.Name = CD.getCompileCommands(InFile).begin()->Output;

    auto &PP = CI.getPreprocessor();
    PP.addPPCallbacks(std::make_unique<MyPPCallbacks>(CI, D));

    return std::make_unique<ASTConsumer>();
  }

public:
  MyAction(CompilationDatabase &CD) : CD(CD), D() {}
  ~MyAction() {
    outs() << "build " << D.Name << ": dyndep |";
    for (auto &It : D.Deps) {
      for (auto &Cmd : CD.getAllCompileCommands()) {
        using namespace llvm::sys;
        if (path::stem(Cmd.Output) == It) {
          outs() << " " << Cmd.Output;
        }
      }
    }
    outs() << "\n";
  }
};

class MyActionFactory : public FrontendActionFactory {
  CompilationDatabase &CD;

public:
  MyActionFactory(CompilationDatabase &CD) : CD(CD) {}
  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<MyAction>(CD);
  }
};
} // namespace

static cl::OptionCategory MyToolCategory("module-dumper options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  clang::IgnoringDiagConsumer DiagConsumer;
  Tool.setDiagnosticConsumer(&DiagConsumer);

  outs() << "ninja_dyndep_version = 1.0\n";

  MyActionFactory AF(OptionsParser.getCompilations());

  return Tool.run(&AF);
}
