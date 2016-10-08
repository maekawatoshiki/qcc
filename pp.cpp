#include "pp.hpp"

std::string Preprocessor::run(std::string source) {
  bool ope = true;
  for(int i = 0; i < source.size(); i++) {
    if(source[i] == '#' && ope) {
      int pp_bgn_pos = i++;
      if(!strncmp(&source[i], "include", 7)) { 
        while(source[i] != '"' && source[i] != '<') i++;
        int name_bgn_pos = ++i;
        while(source[i] != '"' && source[i] != '>') i++;
        int name_count = i - name_bgn_pos; 
        while(source[i] != '\n') i++; i++; // skip this line
        std::string file_name = source.substr(name_bgn_pos, name_count);

        source.erase(pp_bgn_pos, i - pp_bgn_pos);
        source.insert(pp_bgn_pos, [&]() -> std::string {
          std::ifstream ifs_src(default_include_path + file_name);
          if(!ifs_src) { puts("file not found"); return ""; }
          std::istreambuf_iterator<char> it(ifs_src), last;
          std::string src_all(it, last);
          return src_all;
        }());
        source = this->run(source);
      }
    } else if(source[i] == '\n') {
      ope = true;
    } else if(!isblank(source[i])) {
      ope = false; 
    }
  }

  return source;
}
