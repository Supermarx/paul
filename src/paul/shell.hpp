#pragma once

#include <wordexp.h>

#include <paul/parser.hpp>

#include <supermarx/api/client.hpp>
#include <supermarx/util/guard.hpp>

namespace supermarx
{

class shell
{
	std::ostream& os;
	std::istream& is;

	supermarx::api::client c;

private:
	void print_welcome()
	{
		os <<
			R"!(     _____                                                            )!" << std::endl <<
			R"!(    / ___/ __  __ ____   ___   _____ ____ ___   ____ _ _____ _  __  )!" << std::endl <<
			R"!(    \__ \ / / / // __ \ / _ \ / ___// __ `__ \ / __ `// ___/| |/_/)!" << std::endl <<
			R"!(   ___/ // /_/ // /_/ //  __// /   / / / / / // /_/ // /   _>  <   )!" << std::endl <<
			R"!(  /____/ \__,_// .___/ \___//_/   /_/ /_/ /_/ \__,_//_/   /_/|_|  )!" << std::endl <<
			R"!(              /_/                           _____                   __   )!" << std::endl <<
			R"!(                                           / ___/__  ___  ___ ___  / /__ )!" << std::endl <<
			R"!(                                          / /__/ _ \/ _ \(_-</ _ \/ / -_) )!" << std::endl <<
			R"!(                                          \___/\___/_//_/___/\___/_/\__/ )!" << std::endl
			<< std::endl;
	}

	bool ask_input()
	{
		os << "paul> ";

		std::string line;
		if(!std::getline(is, line))
		{
			os << std::endl;
			return false;
		}

		line = std::string("./paul ") + line; // Prepend some arbitrary string for boost::program_options to run normally

		::wordexp_t p;
		guard p_g([&]() { ::wordfree(&p); });

		if(::wordexp(line.c_str(), &p, 0) != 0)
			throw std::runtime_error("Cannot parse line");

		return process(p.we_wordc, p.we_wordv);
	}

	bool process(int argc, char** argv)
	{
		parser::options opt;
		if(parser::read_action(opt, argc, argv) != EXIT_SUCCESS)
			return true;

		return process(opt);
	}

	bool process(parser::options const& opt)
	{
		if(!opt.action)
		{
			os << "Please specify an action, see 'help'." << std::endl;
			return true;
		}

		auto action(*opt.action);
		if(action == "quit")
			return false;
		else
		{
			os << "Unknown action '" << action << "', see 'help'." << std::endl;
		}

		return true;
	}

public:
	shell(std::ostream& _os, std::istream& _is, std::string const& _base_uri)
		: os(_os)
		, is(_is)
		, c("Supermarx/paul 0.1", _base_uri)
	{}

	void interactive()
	{
		print_welcome();
		while(ask_input())
		{}
	}

	static int exec(int argc, char** argv)
	{
		parser::options opt;
		if(parser::read_options(opt, argc, argv) != EXIT_SUCCESS)
			return EXIT_FAILURE;

		shell s(std::cout, std::cin, opt.api_host);
		if(!opt.action)
			s.interactive();
		else
			s.process(opt);

		return EXIT_SUCCESS;
	}
};

}
