# TinyFile

## Build steps
1. `cd <root>/src`
2. Run `make`. For info logs from the executables, use `make DEBUG_LOGS=1` instead.
3. `cd ../bin`
4. Open at least one more terminal (more if multiple clients need to be run) at same location to run the client app.
5. In first terminal, run the service with required params `./TFService --n_sms 5 --sms_size 2048`.
6. On second terminal, run the client app:

    `./TFClient --state (SYNC|ASYNC) [--file (filePath) || --files (fileListPath)] [--out_dir (customOutDir)]`
    
    Argument synopsis:
    * `--state`: client app calls sync or async TinyFile APIs depending on this
    * `--file`: path of the input file to be compressed
    * `--files`: path of a file with the list of input files to be compressed
    * `--out_dir`: path for a custom output directory for compressed files. By default, output directory is `$pwd/output/`.
    * `--no_service`: runs snappy-c compression directly in the client app, for testing purposes. Requires `--sms_size` arg also
    * `--no_service`: shared memory size arg for `--no_service` type of run. Can be used to test if compressed files are almost same size with and without TinyFile.