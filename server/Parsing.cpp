#include "Server.hpp"

Message msg;

std::string extractCommand(std::stringstream &ss)
{
	std::string word;

	ss >> word;

	for (size_t i = 0 ; i < word.size() ; i++)
		word[i] = toupper(word[i]);
	msg.command = word;
	return word;
}

// std::string extractParameters(std::stringstream &ss)
// {
// 	std::string word;

// 	while (ss >> word && word[0] != ':')
// 		msg.params.push_back(word);
// 	// if (word[0] == ':')
// 	// 	msg.trailing = 1;
	
// 	// while (true)
// 	// {
// 	// 	std::streampos pos = ss.tellg();
// 	// 	if (!(ss >> word)) break;

// 	// 	if (word[0] == ':')
// 	// 	{
// 	// 		ss.seekg(pos);  // put it back
// 	// 		break;
// 	// 	}
// 	// 	msg.params.push_back(word);
// 	// }

// 	if (word[0] == ':')
// 		msg.trailing = word.substr(1);
// }

// // USER deeznuts 0 * :Real Name teez \r\n 
// std::string extractTrailing(std::stringstream &ss)
// {

// 	getline()
// 	while (ss >> word)
// 		msg.trailing += word + " ";
// 	msg.trailing.pop_back();
// }

void	splitMessage(std::string unprocessed)
{
	size_t it =	unprocessed.find(" :");
	std::string param = unprocessed.substr(0, it);
	if (it != std::string::npos)
	{
		std::stringstream ss(param);
		extractCommand(ss);
		std::string trailing = unprocessed.substr(it + 2);
	}
	std::string word;

	//extractRest(ss)
	//find " :"
		//substr 0, it
		//handle it in extractparam
		//substr it, \r\n
		//handle it in extracttrail
	//else
	//substr 0, \r\n
	//handle it in extractparam
	// extractParameters(ss);
	// extractTrailing(ss);
}
