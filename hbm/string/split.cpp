
#include "split.h"


namespace hbm {
	namespace string {

		std::vector<std::string> split(std::string text, std::string separator)
		{
		  std::vector<std::string> tokens;

		  if(separator.length() == 0)
		  {
			tokens.push_back(text);
			return tokens;
		  }

		  size_t pos_start=0;

		  while(1)
		  {
			size_t pos_end = text.find(separator, pos_start);
			std::string token = text.substr(pos_start, pos_end-pos_start);
			tokens.push_back(token);
			if(pos_end == std::string::npos) break;
			pos_start = pos_end+separator.length();
		  }
		  return tokens;
		}
	}
}
