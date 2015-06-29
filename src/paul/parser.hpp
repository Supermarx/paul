#pragma once

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace supermarx
{

class parser
{
public:
	parser() = delete;

	struct options
	{
		boost::optional<std::string> action;
		std::string api_host;
		std::string config;
	};

private:
	static int read_commandline(options& opt, int argc, char** argv)
	{
		boost::program_options::options_description o_general("Options");
		o_general.add_options()
				("help,h", "display this message")
				("api,a", boost::program_options::value(&opt.api_host), "api host (default: api.supermarx.nl)")
				("config,C", boost::program_options::value(&opt.config), "path to the configfile (default: ./config.yaml)");

		boost::program_options::variables_map vm;

		boost::program_options::options_description options("Allowed options");
		options.add(o_general);

		try
		{
			boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).run(), vm);
		} catch(boost::program_options::unknown_option &e)
		{
			std::cerr << "Unknown option --" << e.get_option_name() << ", see --help." << std::endl;
			return EXIT_FAILURE;
		}

		try
		{
			boost::program_options::notify(vm);
		} catch(const boost::program_options::required_option &e)
		{
			std::cerr << "You forgot this: " << e.what() << std::endl;
			return EXIT_FAILURE;
		}

		if(vm.count("help"))
		{
			std::cout
					<< "SuperMarx commandline interface Paul. [https://github.com/SuperMarx/paul]" << std::endl
					<< "Usage [1]: ./paul [options] <empty>" << std::endl
					<< "           Drops Paul into an interactive shell." << std::endl
					<< "Usage [2]: ./paul [options] action" << std::endl
					<< std::endl
					<< "Actions:" << std::endl
					<< "  login                 logs an user in" << std::endl
					<< "  quit                  drops from the interactive cell (if applicable)" << std::endl
					<< std::endl
					<< o_general;

			return EXIT_FAILURE;
		}

		if(!vm.count("api"))
			opt.api_host = "https://api.supermarx.nl";

		if(!vm.count("config"))
			opt.config = "./config.yaml";

		return EXIT_SUCCESS;
	}

public:
	static int read_action(options& opt, int argc, char** argv)
	{
		std::string action_tmp;

		boost::program_options::variables_map vm;
		boost::program_options::positional_options_description pos;

		pos.add("action", 1);

		boost::program_options::options_description options("Options");
		options.add_options()
				("action", boost::program_options::value(&action_tmp), "");

		try
		{
			boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).positional(pos).run(), vm);
		} catch(boost::program_options::unknown_option)
		{
			// Do nothing
		}

		try
		{
			boost::program_options::notify(vm);
		} catch(const boost::program_options::required_option &e)
		{
			std::cerr << "You forgot this: " << e.what() << std::endl;
			return EXIT_FAILURE;
		}

		if(vm.count("action"))
			opt.action = action_tmp;

		return EXIT_SUCCESS;
	}

	static int read_options(options& opt, int argc, char** argv)
	{
		int ret_action = read_action(opt, argc, argv);
		if(ret_action != EXIT_SUCCESS)
			return ret_action;

		return read_commandline(opt, argc, argv);
	}
};

}
