# star-submit run directory

Run `star-submit` from **this directory**. SUMS will write generated files (`.csh`, `.list`, `.condor`, `.report`, etc.) here.

## Steps

1. **Build at the project root**
   ```bash
   cd /path/to/star-analysis
   source script/setup.sh && make
   ```

2. **Move to this directory and submit**
   ```bash
   cd job/run
   ./submit.sh
   ```
   By default, `submit.sh` uses the template `../joblist/joblist_run_ana_Phi_forrun.xml`. You can pass another template, e.g. `./submit.sh ../joblist/joblist_run_ana_Lambda.xml`.

   `submit.sh` replaces `__PROJECT_ROOT__` in the template with the actual project path, so the same template works for any user.

## Notes

- Always `cd` into **job/run/** before running `./submit.sh`.
- To use a different joblist template, run e.g. `./submit.sh ../joblist/YourJoblist.xml`.
