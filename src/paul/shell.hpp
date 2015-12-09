#pragma once

#include <wordexp.h>

#include <paul/parser.hpp>

#include <supermarx/api/exception.hpp>
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
			R"!(                                          \___/\___/_//_/___/\___/_/\__/ )!" << std::endl;
	}

	static inline std::vector<std::string> conv_args(int argc, char** argv)
	{
		std::vector<std::string> result;
		result.reserve(argc-1);

		size_t count = static_cast<size_t>(argc);
		for(size_t i = 1; i < count; ++i) // Skip first argument
			result.emplace_back(argv[i]);

		return result;
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
		auto p_g(make_guard([&]() { ::wordfree(&p); }));

		if(::wordexp(line.c_str(), &p, 0) != 0)
			throw std::runtime_error("Cannot parse line");

		return process(conv_args(p.we_wordc, p.we_wordv));
	}

	bool process(std::vector<std::string> const& args)
	{
		parser::options opt;
		if(parser::read_action(opt, args) != EXIT_SUCCESS)
			return true;

		return process(opt);
	}

	bool process(parser::options const& opt)
	{
		try
		{
			if(!opt.action)
			{
				os << "Please specify an action, see 'help'." << std::endl;
				return true;
			}

			auto action(*opt.action);
			if(action == "quit")
				return false;
			else if(action == "login")
			{
				std::string username, password;

				os << "Username: ";
				std::getline(is, username);

				if(username == "")
				{
					os << "Nothing entered, bumping back to shell." << std::endl;
					return true;
				}

				os << "Password: ";
				std::getline(is, password);

				if(password == "")
				{
					os << "Nothing entered, bumping back to shell." << std::endl;
					return true;
				}

				os << "Logging in... (this may take a while)" << std::endl;
				c.promote(username, password);

				os << "Logged in successfully as '" << username << "'." << std::endl;
			}
			else if(action == "find")
			{

			}
			else
			{
				os << "Unknown action '" << action << "', see 'help'." << std::endl;
			}
		} catch(api::exception e)
		{
			os << "Uncaught API exception: " << api::exception_message(e) << " (" << e << ")" << std::endl;
		}

		return true;
	}

public:
	shell(std::ostream& _os, std::istream& _is, std::string const& _base_uri)
		: os(_os)
		, is(_is)
		, c(_base_uri, "Supermarx/paul 0.1")
	{}

	void interactive()
	{
		print_welcome();

		while(ask_input())
		{}
	}

	static int exec(int argc, char** argv)
	{
		return exec(conv_args(argc, argv));
	}

	static int exec(std::vector<std::string> const& args)
	{
		parser::options opt;
		if(parser::read_options(opt, args) != EXIT_SUCCESS)
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
