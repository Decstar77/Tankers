ServerFiles="server/server_main.cpp"
SharedFiles=""

# Find all shared .cpp files
for f in shared/*.cpp; do
    SharedFiles="$SharedFiles $f"
done

# Compile the files using g++
g++ $ServerFiles $SharedFiles -o ser

# Optional: print a message to indicate success
echo "Compilation successful! Executable is named 'ser'"
