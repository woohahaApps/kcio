/*
 * Copyright (c) 2009  Daniel Elstner <daniel.kitta@gmail.com>
 *
 * This file is part of KC-Mill.
 *
 * KC-Mill is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KC-Mill is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include "inputwindow.h"
#include "serialport.h"
#include <glibmm.h>
#include <gtkmm/main.h>
#include <glib.h>
#include <locale>
#include <memory>
#include <stdexcept>
#include <vector>
#include <glib/gi18n.h>

namespace
{

class CmdlineOptions
{
private:
  std::vector<std::string> filenames_;
  Glib::OptionGroup        group_;
  Glib::OptionContext      context_;

public:
  CmdlineOptions();
  ~CmdlineOptions();

  Glib::OptionContext& context() { return context_; }
  std::string& portname() { return filenames_.front(); } // container is never empty
};

CmdlineOptions::CmdlineOptions()
:
  filenames_ (1, "/dev/ttyS0"),
  group_     ("kckeyb", "KC-Keyboard"),
  context_   ()
{
  Glib::OptionEntry entry;

  entry.set_long_name(G_OPTION_REMAINING);
  entry.set_arg_description(N_("[PORT]"));
  group_.add_entry_filename(entry, filenames_);

  group_.set_translation_domain(PACKAGE_TARNAME);
  context_.set_main_group(group_);
}

CmdlineOptions::~CmdlineOptions()
{}

static void init_locale()
{
  try
  {
    std::locale::global(std::locale(""));
  }
  catch (const std::runtime_error& ex)
  {
    g_warning("Locale: %s", ex.what());
  }
  bindtextdomain(PACKAGE_TARNAME, KCMILL_LOCALEDIR);
#if HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(PACKAGE_TARNAME, "UTF-8");
#endif
  textdomain(PACKAGE_TARNAME);
}

} // anonymous namespace

int main(int argc, char** argv)
{
  try
  {
    init_locale();
    std::auto_ptr<CmdlineOptions> options (new CmdlineOptions());
    Gtk::Main kit (argc, argv, options->context());
    Glib::set_application_name("KC-Keyboard");

    KC::SerialPort port (options->portname());
    options.reset();
    KC::InputWindow window;

    Gtk::Main::run(window);

    port.close();
    return 0;
  }
  catch (const Glib::OptionError& error)
  {
    const Glib::ustring what = error.what();
    g_printerr("%s: %s\n", g_get_prgname(), what.c_str());
  }
  catch (const Glib::Error& error)
  {
    const Glib::ustring what = error.what();
    g_error("unhandled exception: %s", what.c_str());
  }
  catch (const std::exception& ex)
  {
    g_error("unhandled exception: %s", ex.what());
  }
  catch (...)
  {
    g_error("unhandled exception: (type unknown)");
  }
  return 1;
}
