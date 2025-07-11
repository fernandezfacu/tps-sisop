#!/bin/bash

clear
chmod +x integration_tests/empty_fs.sh
./integration_tests/empty_fs.sh
chmod +x integration_tests/create_file.sh
./integration_tests/create_file.sh
chmod +x integration_tests/rm_file.sh
./integration_tests/rm_file.sh
chmod +x integration_tests/r_w_file.sh
./integration_tests/r_w_file.sh
chmod +x integration_tests/stat_file.sh
./integration_tests/stat_file.sh
chmod +x integration_tests/create_dir.sh
./integration_tests/create_dir.sh
chmod +x integration_tests/read_dir.sh
./integration_tests/read_dir.sh
chmod +x integration_tests/rm_dir.sh
./integration_tests/rm_dir.sh