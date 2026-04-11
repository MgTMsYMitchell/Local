# CMakeLists.txt modified for SQLite3

# Existing content above this line is assumed to be unchanged

# Update SQLite3 version referenced message
if(SQLite3_FOUND)
    message(STATUS "Using system SQLite3")
    set(RUNE_SQLITE_TARGET SQLite::SQLite3)
else()
    # The rest of the fallback amalgamation logic remains unchanged
end_if()

# Existing content below this line is assumed to be unchanged