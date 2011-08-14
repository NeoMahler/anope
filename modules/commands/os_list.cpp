/* OperServ core functions
 *
 * (C) 2003-2011 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandOSChanList : public Command
{
 public:
	CommandOSChanList(Module *creator) : Command(creator, "operserv/chanlist", 0, 2)
	{
		this->SetDesc(_("Lists all channel records"));
		this->SetSyntax(_("[{\037pattern\037 | \037nick\037} [\037SECRET\037]]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &pattern = !params.empty() ? params[0] : "";
		const Anope::string &opt = params.size() > 1 ? params[1] : "";
		std::list<ChannelModeName> Modes;
		User *u2;

		if (!opt.empty() && opt.equals_ci("SECRET"))
		{
			Modes.push_back(CMODE_SECRET);
			Modes.push_back(CMODE_PRIVATE);
		}

		if (!pattern.empty() && (u2 = finduser(pattern)))
		{
			source.Reply(_("\002%s\002 channel list:\n"
					"Name                 Users Modes   Topic"), u2->nick.c_str());

			for (UChannelList::iterator uit = u2->chans.begin(), uit_end = u2->chans.end(); uit != uit_end; ++uit)
			{
				ChannelContainer *cc = *uit;

				if (!Modes.empty())
					for (std::list<ChannelModeName>::iterator it = Modes.begin(), it_end = Modes.end(); it != it_end; ++it)
						if (!cc->chan->HasMode(*it))
							continue;

				source.Reply(_("%-20s  %4d +%-6s %s"), cc->chan->name.c_str(), cc->chan->users.size(), cc->chan->GetModes(true, true).c_str(), !cc->chan->topic.empty() ? cc->chan->topic.c_str() : "");
			}
		}
		else
		{
			source.Reply(_("Channel list:\n"
					"Name                 Users Modes   Topic"));

			for (channel_map::const_iterator cit = ChannelList.begin(), cit_end = ChannelList.end(); cit != cit_end; ++cit)
			{
				Channel *c = cit->second;

				if (!pattern.empty() && !Anope::Match(c->name, pattern))
					continue;
				if (!Modes.empty())
					for (std::list<ChannelModeName>::iterator it = Modes.begin(), it_end = Modes.end(); it != it_end; ++it)
						if (!c->HasMode(*it))
							continue;

				source.Reply(_("%-20s  %4d +%-6s %s"), c->name.c_str(), c->users.size(), c->GetModes(true, true).c_str(), !c->topic.empty() ? c->topic.c_str() : "");
			}
		}

		source.Reply(_("End of channel list."));
		return;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Lists all channels currently in use on the IRC network, whether they\n"
				"are registered or not.\n"
				"If \002pattern\002 is given, lists only channels that match it. If a nickname\n"
				"is given, lists only the channels the user using it is on. If SECRET is\n"
				"specified, lists only channels matching \002pattern\002 that have the +s or\n"
				"+p mode."));
		return true;
	}
};

class CommandOSUserList : public Command
{
 public:
	CommandOSUserList(Module *creator) : Command(creator, "operserv/userlist", 0, 2)
	{
		this->SetDesc(_("Lists all user records"));
		this->SetSyntax(_("[{\037pattern\037 | \037channel\037} [\037INVISIBLE\037]]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &pattern = !params.empty() ? params[0] : "";
		const Anope::string &opt = params.size() > 1 ? params[1] : "";
		Channel *c;
		std::list<UserModeName> Modes;

		if (!opt.empty() && opt.equals_ci("INVISIBLE"))
			Modes.push_back(UMODE_INVIS);

		if (!pattern.empty() && (c = findchan(pattern)))
		{
			source.Reply(_("\002%s\002 users list:\n"
					"Nick                 Mask"), pattern.c_str());

			for (CUserList::iterator cuit = c->users.begin(), cuit_end = c->users.end(); cuit != cuit_end; ++cuit)
			{
				UserContainer *uc = *cuit;

				if (!Modes.empty())
					for (std::list<UserModeName>::iterator it = Modes.begin(), it_end = Modes.end(); it != it_end; ++it)
						if (!uc->user->HasMode(*it))
							continue;

				source.Reply(_("%-20s %s@%s"), uc->user->nick.c_str(), uc->user->GetIdent().c_str(), uc->user->GetDisplayedHost().c_str());
			}
		}
		else
		{
			source.Reply(_("Users list:\n"
					"Nick                 Mask"));

			for (Anope::insensitive_map<User *>::iterator it = UserListByNick.begin(); it != UserListByNick.end(); ++it)
			{
				User *u2 = it->second;

				if (!pattern.empty())
				{
					Anope::string mask = u2->nick + "!" + u2->GetIdent() + "@" + u2->GetDisplayedHost();
					if (!Anope::Match(mask, pattern))
						continue;
					if (!Modes.empty())
						for (std::list<UserModeName>::iterator mit = Modes.begin(), mit_end = Modes.end(); mit != mit_end; ++mit)
							if (!u2->HasMode(*mit))
								continue;
				}
				source.Reply(_("%-20s %s@%s"), u2->nick.c_str(), u2->GetIdent().c_str(), u2->GetDisplayedHost().c_str());
			}
		}

		source.Reply(_("End of users list."));
		return;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Lists all users currently online on the IRC network, whether their\n"
				"nick is registered or not.\n"
				" \n"
				"If \002pattern\002 is given, lists only users that match it (it must be in\n"
				"the format nick!user@host). If \002channel\002 is given, lists only users\n"
				"that are on the given channel. If INVISIBLE is specified, only users\n"
				"with the +i flag will be listed."));
		return true;
	}
};

class OSList : public Module
{
	CommandOSChanList commandoschanlist;
	CommandOSUserList commandosuserlist;

 public:
	OSList(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, CORE),
		commandoschanlist(this), commandosuserlist(this)
	{
		this->SetAuthor("Anope");

		ModuleManager::RegisterService(&commandoschanlist);
		ModuleManager::RegisterService(&commandosuserlist);
	}
};

MODULE_INIT(OSList)