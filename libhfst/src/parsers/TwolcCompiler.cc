//   This program is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, version 3 of the Licence.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "HfstTwolcDefs.h"
#include "io_src/InputReader.h"
#include "grammar_defs.h"
#include "rule_src/TwolCGrammar.h"
#include "rule_src/OtherSymbolTransducer.h"
#include "TwolcCompiler.h"

#ifdef PYTHON_BINDINGS
#include "pybind11/pybind11.h"
#include "pybind11/iostream.h"
namespace py = pybind11;
#endif

namespace hfst {
  namespace twolcpre1 {
    int parse();
    void set_input(std::istream & istr);
    void set_output(std::ostream & ostr);
    void reset_lexer();
    void reset_parser();
    void set_warning_stream(std::ostream & ostr);
    void set_error_stream(std::ostream & ostr);
  }
}

namespace hfst {
  namespace twolcpre2 {
    int parse();
    void set_input(std::istream & istr);
    void complete_alphabet(void);
    const HandyDeque<std::string> & get_total_alphabet_symbol_queue();
    const HandyDeque<std::string> & get_non_alphabet_symbol_queue();
    void reset_lexer();
    void reset_parser();
    void set_warning_stream(std::ostream & ostr);
    void set_error_stream(std::ostream & ostr);
  }
}

namespace hfst {
  namespace twolcpre3 {
    int parse();
    void set_input(std::istream & istr);
    void set_grammar(TwolCGrammar * grammar);
    TwolCGrammar * get_grammar();
    void set_silent(bool val);
    void set_verbose(bool val);
    void reset_parser();
    void set_warning_stream(std::ostream & ostr);
    void set_error_stream(std::ostream & ostr);
  }
}

namespace hfst {
  namespace twolc {

    int TwolcCompiler::compile
    (const std::string & inputfile, const std::string & outputfile,
     bool silent, bool verbose, bool resolve_left_conflicts,
     bool resolve_right_conflicts, hfst::ImplementationType type,
     std::ostream * ostr)
    {
      // Reset previous values
      hfst::twolcpre1::reset_lexer();
      hfst::twolcpre1::reset_parser();

      // (1) Preprocessing
      std::ifstream istr(inputfile.c_str());
      hfst::twolcpre1::set_input(istr);
      std::ostringstream oss1;
      hfst::twolcpre1::set_output(oss1);
      if (ostr != NULL)
	{
	  hfst::twolcpre1::set_warning_stream(*ostr);
	  hfst::twolcpre1::set_error_stream(*ostr);
	}

      try
	{
	  int retval = hfst::twolcpre1::parse();
	  if (retval != 0)
	    {
	      return retval;
	    }
	}
      catch(const HfstException & e)
	{
	  std::cerr << e.what() << std::endl;
	  return -1;
	}

      // Reset previous values
      hfst::twolcpre2::reset_lexer();
      hfst::twolcpre2::reset_parser();

      // (2) Preprocessing
      std::istringstream iss1(oss1.str());
      hfst::twolcpre2::set_input(iss1);
      if (ostr != NULL)
	{
	  hfst::twolcpre2::set_warning_stream(*ostr);
	  hfst::twolcpre2::set_error_stream(*ostr);
	}

      try
	{
	  int retval = hfst::twolcpre2::parse();
	  if (retval != 0)
	    {
	      return retval;
	    }
	}
      catch(const HfstException & e)
	{
	  std::cerr << e.what() << std::endl;
	  return -1;
	}

      hfst::twolcpre2::complete_alphabet();

      std::ostringstream oss2;
      oss2 << hfst::twolcpre2::get_total_alphabet_symbol_queue() << " ";
      oss2 << hfst::twolcpre2::get_non_alphabet_symbol_queue();

      // Reset previous values
      hfst::twolcpre3::reset_parser();

      // (3) Compilation
      try
	{
	  std::istringstream iss2(oss2.str());
	  hfst::twolcpre3::set_input(iss2);
	  if (ostr != NULL)
	    {
	      hfst::twolcpre3::set_warning_stream(*ostr);
	      hfst::twolcpre3::set_error_stream(*ostr);
	    }

	  OtherSymbolTransducer::set_transducer_type(type);
	  hfst::twolcpre3::set_silent(silent);
	  hfst::twolcpre3::set_verbose(verbose);

	  TwolCGrammar twolc_grammar(silent,
				     verbose,
				     resolve_left_conflicts,
				     resolve_right_conflicts);
	  hfst::twolcpre3::set_grammar(&twolc_grammar);
	  int exit_code = hfst::twolcpre3::parse();
	  if (exit_code != 0)
	    { return exit_code; }

	  HfstOutputStream out
	    (outputfile,type);
	  hfst::twolcpre3::get_grammar()->compile_and_store(out);

	  return exit_code;
	}
      catch (const HfstException e)
	{
	  std::cerr << "This is an hfst interface bug:" << std::endl
		    << e() << std::endl;
	  return -1;
	}
      catch (const char * s)
	{
	  std::cerr << "This is an a bug probably from sfst:" << std::endl
		    << s << std::endl;
	  return -1;
	}

    }

    std::vector<hfst::HfstTransducer> TwolcCompiler::compile_file_and_get_storable_rules
    (const std::string & inputfile,
     bool silent, bool verbose, bool resolve_left_conflicts,
     bool resolve_right_conflicts, hfst::ImplementationType type,
     std::ostream * ostr)
    {
      // Reset previous values
      hfst::twolcpre1::reset_lexer();
      hfst::twolcpre1::reset_parser();

      // (1) Preprocessing
      std::ifstream istr(inputfile.c_str());
      hfst::twolcpre1::set_input(istr);
      std::ostringstream oss1;
      hfst::twolcpre1::set_output(oss1);
      if (ostr != NULL)
	{
	  hfst::twolcpre1::set_warning_stream(*ostr);
	  hfst::twolcpre1::set_error_stream(*ostr);
	}

      try
	{
	  int retval = hfst::twolcpre1::parse();
	  if (retval != 0)
	    {
	      //return retval;
	      throw "ERROR";
	    }
	}
      catch(const HfstException & e)
	{
	  throw e;
	  //std::cerr << e.what() << std::endl;
	  //return -1;
	}

      // Reset previous values
      hfst::twolcpre2::reset_lexer();
      hfst::twolcpre2::reset_parser();

      // (2) Preprocessing
      std::istringstream iss1(oss1.str());
      hfst::twolcpre2::set_input(iss1);
      if (ostr != NULL)
	{
	  hfst::twolcpre2::set_warning_stream(*ostr);
	  hfst::twolcpre2::set_error_stream(*ostr);
	}

      try
	{
	  int retval = hfst::twolcpre2::parse();
	  if (retval != 0)
	    {
	      //return retval;
	      throw "ERROR";
	    }
	}
      catch(const HfstException & e)
	{
	  //std::cerr << e.what() << std::endl;
	  //return -1;
	  throw e;
	}

      hfst::twolcpre2::complete_alphabet();

      std::ostringstream oss2;
      oss2 << hfst::twolcpre2::get_total_alphabet_symbol_queue() << " ";
      oss2 << hfst::twolcpre2::get_non_alphabet_symbol_queue();

      // Reset previous values
      hfst::twolcpre3::reset_parser();

      // (3) Compilation
      try
	{
	  std::istringstream iss2(oss2.str());
	  hfst::twolcpre3::set_input(iss2);
	  if (ostr != NULL)
	    {
	      hfst::twolcpre3::set_warning_stream(*ostr);
	      hfst::twolcpre3::set_error_stream(*ostr);
	    }

	  OtherSymbolTransducer::set_transducer_type(type);
	  hfst::twolcpre3::set_silent(silent);
	  hfst::twolcpre3::set_verbose(verbose);

	  TwolCGrammar twolc_grammar(silent,
				     verbose,
				     resolve_left_conflicts,
				     resolve_right_conflicts);
	  hfst::twolcpre3::set_grammar(&twolc_grammar);
	  int exit_code = hfst::twolcpre3::parse();
	  if (exit_code != 0)
	    {
	      // return exit_code;
	      throw "ERROR";
	    }

	  return hfst::twolcpre3::get_grammar()->compile_and_get_storable_rules();

	}
      catch (const HfstException e)
	{
	  throw e;
	  //std::cerr << "This is an hfst interface bug:" << std::endl
	  //	    << e() << std::endl;
	  //return -1;
	}
      catch (const char * s)
	{
	  throw s;
	  //std::cerr << "This is an a bug probably from sfst:" << std::endl
	  //	    << s << std::endl;
	  //return -1;
	}

    }

    std::vector<hfst::HfstTransducer> TwolcCompiler::compile_script_and_get_storable_rules
    (const std::string & script,
     bool silent, bool verbose, bool resolve_left_conflicts,
     bool resolve_right_conflicts, hfst::ImplementationType type,
     std::ostream * ostr)
    {
      // Reset previous values
      hfst::twolcpre1::reset_lexer();
      hfst::twolcpre1::reset_parser();

      // (1) Preprocessing
      //std::ifstream istr(inputfile.c_str());
      std::istringstream istr(script);
      hfst::twolcpre1::set_input(istr);
      std::ostringstream oss1;
      hfst::twolcpre1::set_output(oss1);
      if (ostr != NULL)
	{
	  hfst::twolcpre1::set_warning_stream(*ostr);
	  hfst::twolcpre1::set_error_stream(*ostr);
	}

      try
	{
	  int retval = hfst::twolcpre1::parse();
	  if (retval != 0)
	    {
	      //return retval;
	      throw "ERROR";
	    }
	}
      catch(const HfstException & e)
	{
	  throw e;
	  //std::cerr << e.what() << std::endl;
	  //return -1;
	}

      // Reset previous values
      hfst::twolcpre2::reset_lexer();
      hfst::twolcpre2::reset_parser();

      // (2) Preprocessing
      std::istringstream iss1(oss1.str());
      hfst::twolcpre2::set_input(iss1);
      if (ostr != NULL)
	{
	  hfst::twolcpre2::set_warning_stream(*ostr);
	  hfst::twolcpre2::set_error_stream(*ostr);
	}

      try
	{
	  int retval = hfst::twolcpre2::parse();
	  if (retval != 0)
	    {
	      //return retval;
	      throw "ERROR";
	    }
	}
      catch(const HfstException & e)
	{
	  //std::cerr << e.what() << std::endl;
	  //return -1;
	  throw e;
	}

      hfst::twolcpre2::complete_alphabet();

      std::ostringstream oss2;
      oss2 << hfst::twolcpre2::get_total_alphabet_symbol_queue() << " ";
      oss2 << hfst::twolcpre2::get_non_alphabet_symbol_queue();

      // Reset previous values
      hfst::twolcpre3::reset_parser();

      // (3) Compilation
      try
	{
	  std::istringstream iss2(oss2.str());
	  hfst::twolcpre3::set_input(iss2);
	  if (ostr != NULL)
	    {
	      hfst::twolcpre3::set_warning_stream(*ostr);
	      hfst::twolcpre3::set_error_stream(*ostr);
	    }

	  OtherSymbolTransducer::set_transducer_type(type);
	  hfst::twolcpre3::set_silent(silent);
	  hfst::twolcpre3::set_verbose(verbose);

	  TwolCGrammar twolc_grammar(silent,
				     verbose,
				     resolve_left_conflicts,
				     resolve_right_conflicts);
	  hfst::twolcpre3::set_grammar(&twolc_grammar);
	  int exit_code = hfst::twolcpre3::parse();
	  if (exit_code != 0)
	    {
	      // return exit_code;
	      throw "ERROR";
	    }

	  return hfst::twolcpre3::get_grammar()->compile_and_get_storable_rules();

	}
      catch (const HfstException e)
	{
	  throw e;
	  //std::cerr << "This is an hfst interface bug:" << std::endl
	  //	    << e() << std::endl;
	  //return -1;
	}
      catch (const char * s)
	{
	  throw s;
	  //std::cerr << "This is an a bug probably from sfst:" << std::endl
	  //	    << s << std::endl;
	  //return -1;
	}

    }

    void TwolcCompiler::print_message(const std::string & msg)
    {
#ifdef PYTHON_BINDINGS
      auto d = py::dict();
      d["file"] = py::module::import("sys").attr("stderr");
      //d["end"] = "";
      py::print(msg.c_str(), **d);
#else
      std::cerr << msg << std::endl;
#endif
    }

  } // namespace twolc
} // namespace hfst
