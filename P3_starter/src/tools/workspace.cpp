// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {

    if (user_path.empty()){
        *reason = "empty path";
        return false;
    }
    if (user_path.find('\0') != std::string_view::npos){
        *reason = "Null found";
        return false;
    }
fs::path pa{std::string(user_path)};

    if (pa.is_absolute()||pa.has_root_path()){
        *reason = "path is absolute or root which is not allowd and bad";
        return false;
    }
    pa = pa.lexically_normal();

    if(*pa.begin() ==("..")){
        *reason = "path is leaves acceptable area which is not allowd and bad";
    return false;
}
    if(pa =="."){
        *out = root();
}
    else{
        *out = root()/pa;
}
return true;


}
