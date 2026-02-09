# STAR analysis framework

STAR PicoDst-based analysis using the StChain/StMaker pattern: analysis macros drive the event loop, and custom Makers (compiled as shared libraries) perform the physics logic.

## Philosophy

- **No custom event loop in macros.** The macro builds a `StChain`, calls `chain->Init()`, then runs `chain->Make(i)` in a loop and `chain->Finish()`. The framework controls the loop.
- **Physics in StMaker subclasses.** Each analysis has a Maker (e.g. `StPhiMaker`) that implements `Init()`, `Make()`, `Clear()`, `Finish()`. Histograms are created in `DeclareHistograms()` and written in `WriteHistograms()`.
- **Makers are compiled.** Maker code lives in `StMaker/StXXXMaker/` and is built into `lib/libStXXXMaker.so`. This keeps heavy logic out of the interpreter and allows reuse.
- **Macros run under root4star.** The entry point is a ROOT macro invoked with `root4star -b -q "..."`. A small shell script (`script/run_ana_XXX.sh`) sets the environment and calls the macro.
- **YAML-driven config.** Cuts and histogram definitions are read from YAML via `ConfigManager`, so you can change them without recompiling Makers when using existing cut/hist keys.

## Why two macros per analysis (run_ana_XXX.C and ana_XXX.C)

ROOT's CINT interpreter does **not** resolve symbols from dynamically loaded shared libraries. So a single macro that does `gSystem->Load("libStPhiMaker.so")` and then `new StPhiMaker(...)` fails with "declared but not defined."

The fix is to **compile** the macro that uses the Maker with ACLiC (`.L ana_Phi.C+`) and link it against the Maker library. That requires:

1. **Before** compilation: load STAR libs, load `libStXXXMaker.so`, and tell ACLiC to link against it (`gSystem->AddLinkedLibs(...)`).
2. **Then** compile and load the analysis macro (`.L ana_XXX.C+`).
3. **Then** call the analysis function (e.g. `ana_Phi(...)`).

Because `root4star -q` accepts only **one** macro invocation, that sequence is implemented in a **runner** macro, and the actual chain/event loop lives in a **second** macro that gets compiled:

- **run_ana_XXX.C** — Loads STAR and project libs, sets `AddLinkedLibs`, compiles `ana_XXX.C+`, and calls `ana_XXX(...)`.
- **ana_XXX.C** — Builds `StChain`, adds `StPicoDstMaker` and your Maker, runs the event loop. This file is compiled by ACLiC and linked to `libStXXXMaker.so`.

So each new analysis uses this two-file pattern on purpose; the runner is analysis-specific and not shared.

## Directory layout

Only **template/sample** content under `config/` and `job/joblist/` is tracked; built artifacts under `lib/` are git-ignored.

| Directory | Description |
|-----------|-------------|
| **analysis/** | ROOT macros: `run_ana_XXX.C` (runner: loads libs, compiles `ana_XXX.C+`, calls analysis) and `ana_XXX.C` (StChain + event loop). One pair per analysis (e.g. Lambda, Phi). |
| **config/** | YAML configs. **Templates/samples only** tracked. Subdirs: `mainconf/` (main YAML that includes the rest), `maker/`, `hist/`, `cuts/` (event, track, pid, v0reco, mixing), `analysis/`, `picoDstList/` (input file lists; user lists are typically untracked). |
| **include/** | Framework headers: `ConfigManager.h`, `HistManager.h`, cut configs (`cuts/*.h`). Used by StMaker and `src/`. |
| **job/** | Job submission: `job/joblist/` = **template** job XMLs (tracked); `job/run/` = submit directory (`submit.sh`, generated/copied files). Files under `job/run/*.xml` and SUMS outputs are git-ignored. |
| **lib/** | Built shared libraries (`libStarAnaConfig.so`, `libStXXXMaker.so`). **Contents git-ignored**; produced by `make`. |
| **StMaker/** | One subdir per Maker (e.g. `StLambdaMaker/`, `StPhiMaker/`). Each has `.h` and `.cxx`; built into `lib/libStXXXMaker.so`. |
| **script/** | Environment and run scripts: `setup.sh` (starver), `run_ana_Lambda.sh`, `run_ana_Phi.sh`, `job_template.xml`, and helpers (e.g. `get_file_list_*.sh`). |

## Prerequisites and setup

- **STAR environment**: use `starver` (e.g. `starver SL24y`). Set it via `source script/setup.sh` from the project root; adjust the version in `script/setup.sh` if your site uses another one.
- **Build tools**: ROOT, gcc, and CMake (for yaml-cpp). The framework is intended to be buildable with any starver environment (subject to future development).

From the project root:

```bash
source script/setup.sh
```

## Submodules

This repository uses a git submodule for YAML parsing:

- **src/third_party/yaml-cpp** — [jbeder/yaml-cpp](https://github.com/jbeder/yaml-cpp); required to build `libStarAnaConfig.so`.

**After cloning**, initialize and update the submodule:

```bash
git submodule update --init --recursive
```

If you clone with recursive submodules, you can skip the above:

```bash
git clone --recurse-submodules <repository-url>
```

Without the submodule populated, `make` will fail when building the config library.

## Build

From the project root:

```bash
source script/setup.sh
make
```

This builds `lib/libStarAnaConfig.so`, `lib/libStPhiMaker.so`, and `lib/libStLambdaMaker.so`. The Makefile uses `$STAR` and `root-config`; other Makers need their own targets (see "Adding a new analysis" below).

## How to run

### Lambda analysis example (local with root4star)

Run from the **project root**:

```bash
./script/run_ana_Lambda.sh [inputList] [outputRoot] [jobid] [nEvents] [configPath]
```

Defaults:

- `inputList`   = `config/picoDstList/auau19GeV_lambda.list`
- `outputRoot`  = `rootfile/lambda_auau19/lambda_auau19_ana_Lambda.root`
- `jobid`       = `0`
- `nEvents`     = `-1` (all events)
- `configPath`  = (default main config; omit to use `config/mainconf/main_lambda.yaml`)

Example (first 100 events):

```bash
./script/run_ana_Lambda.sh config/picoDstList/auau19GeV_lambda.list rootfile/lambda_auau19/out.root 0 100
```

With a custom main config (5th argument):

```bash
./script/run_ana_Lambda.sh config/picoDstList/auau19GeV_lambda.list rootfile/lambda_auau19/out.root 0 -1 config/mainconf/main_lambda.yaml
```

The script sets `LD_LIBRARY_PATH` and runs:

```bash
root4star -b -q "analysis/run_ana_Lambda.C(\"$INPUT\",\"$OUTPUT\",\"$JOBID\",$NEVENTS[, \"$CONFIG\"])"
```

`run_ana_Lambda.C` loads STAR libs, `libStarAnaConfig.so`, and `libStLambdaMaker.so`, compiles `ana_Lambda.C+`, and calls `ana_Lambda(...)`.

### Batch (star-submit)

1. **Build first** at the project root: `source script/setup.sh && make`.
2. **Submit from** `job/run/`:
   ```bash
   cd job/run
   ./submit.sh ../joblist/joblist_run_ana_Lambda.xml
   ```
   Use another template (e.g. `../joblist/joblist_run_ana_Phi_forrun.xml`) if needed.

`submit.sh` replaces `__PROJECT_ROOT__` in the template with the actual project path so SandBox paths work for any user. If the template uses hardcoded stdout/stderr/output URLs, copy the template and set those URLs to your own log and output directories. See `job/run/README.md` for details.

## Adding a new analysis (new StMaker)

Each analysis has:

1. A Maker in `StMaker/StXXXMaker/` (e.g. `StLambdaMaker`).
2. A shared library `lib/libStXXXMaker.so` (e.g. `libStLambdaMaker.so`).
3. Two macros: `analysis/run_ana_XXX.C` and `analysis/ana_XXX.C`.
4. A run script: `script/run_ana_XXX.sh`.

### 1. Maker code

- Create `StMaker/StXXXMaker/StXXXMaker.h` and `StMaker/StXXXMaker/StXXXMaker.cxx`.
- Subclass `StMaker`; implement `Init()`, `Make()`, `Clear()`, `Finish()`.
- Create histograms in `DeclareHistograms()` and write them in `WriteHistograms()` (e.g. in `Finish()`).
- Use `ConfigManager::GetInstance()` for cuts and `GetHistConfigPath()` for the hist YAML path if your Maker uses config.
- Use `StPicoDstMaker` to access PicoDst; get the chain from it as needed.

### 2. Build the shared library

- In the **Makefile**, add a target for `lib/libStXXXMaker.so` (same pattern as `libStPhiMaker.so` / `libStLambdaMaker.so`). If the Maker uses config, link against `libStarAnaConfig`.
- Run `make` so that `lib/libStXXXMaker.so` exists.

### 3. Runner macro (run_ana_XXX.C)

- Copy `analysis/run_ana_Phi.C` (or `run_ana_Lambda.C`) to `analysis/run_ana_XXX.C`.
- Replace the analysis name in: function name, library `libStXXXMaker.so`, macro `ana_XXX.C+`, and call `ana_XXX(...)`. If using config, keep loading `libStarAnaConfig.so` and pass the config path as the 5th argument where applicable.

### 4. Analysis macro (ana_XXX.C)

- Copy `analysis/ana_Phi.C` or `analysis/ana_Lambda.C` to `analysis/ana_XXX.C`.
- Before building the chain, load the main config: `ConfigManager::GetInstance().LoadConfig(mainConfigPath)` (resolve `mainConfigPath` from the 5th argument or default to e.g. `config/mainconf/main_XXX.yaml`).
- Replace the Maker type and variable names; keep the same signature so the runner can call it (including optional 5th parameter `configPath` if used).

### 5. Run script (run_ana_XXX.sh)

- Copy `script/run_ana_Phi.sh` or `script/run_ana_Lambda.sh` to `script/run_ana_XXX.sh`.
- Point to `run_ana_XXX.C` and set default input, output, and config paths.

## Creating your own config

- **Main config**: Copy `config/mainconf/main_lambda.yaml` (or `main_phi.yaml`) to e.g. `config/mainconf/main_myanalysis.yaml`. It references (paths are relative to `config/`):
  - **Cuts**: `event`, `track`, `pid`, `v0`, `mixing`, and optionally analysis-specific keys (e.g. `phi`, `lambda`).
  - **Maker**: e.g. `lambda: maker/maker_lambda.yaml`.
  - **Hist**: `hist: hist/hist_lambda.yaml`.
  - **Analysis info**: `analysis: analysis/analysis_info.yaml`.
- **Maker config**: Add e.g. `config/maker/maker_my.yaml` and reference it in the main config under the key your Maker expects. Makers read cuts via `ConfigManager::GetInstance().GetXXXCuts()` and the hist path via `GetHistConfigPath()`.
- **Hist config**: Add e.g. `config/hist/hist_my.yaml` with the same structure as existing hist YAMLs (`axes`, `histograms`). Set the `hist` key in the main config to this file.
- **New cut type**: If you need a new cut category, add a new key in the main YAML, a new `XxxCutConfig` in `include/cuts/` and `src/cuts/`, and register it in `ConfigManager`. For a new analysis that only uses existing event/track/pid/v0/mixing and maker keys, copying and editing the existing YAMLs under `config/cuts/`, `config/maker/`, and `config/hist/` is enough.

## Creating a joblist

- **Templates** live in `job/joblist/` (e.g. `joblist_run_ana_Lambda.xml`, `joblist_run_ana_Phi_forrun.xml`). These are the tracked templates.
- **Copy and edit**: Copy a template to a new name (e.g. `joblist_run_ana_MyMaker.xml`). Then:
  - **command**: Set the macro and arguments (e.g. `run_ana_XXX.C`, `$FILELIST`, output path under `$SCRATCH`, `$JOBID`, `-1`, and config path).
  - **SandBox**: Keep `File` entries for `analysis/`, `lib/`, `config/`, `include/`, `StMaker/`. Use `__PROJECT_ROOT__` in paths so `job/run/submit.sh` can substitute the project root.
  - **stdout / stderr / output toURL**: Set to your log and output directories (these are user-specific and not replaced by `submit.sh`).
  - **input**: Set the catalog or file list and `nFiles` as needed.
- **Submit**: From `job/run`, run `./submit.sh ../joblist/YourJoblist.xml`. Ensure `make` has been run so `lib/` contains the required `.so` files.
