script_dir=$(dirname "$0")

cd $script_dir

python3 -m http.server
