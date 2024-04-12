#pragma once
#include "Command.h"

/**
 * \ingroup TortoiseProc
 * Shows the DST Add Worktree dialog.
 */
class DstWorktreeCommand : public Command
{
	bool m_bRemove;

public:
	DstWorktreeCommand(bool bRemove = false)
		: m_bRemove(bRemove)
	{
	}
	/**
	 * Executes the command.
	 */
	bool Execute() override;
};

class DstOpenSolutionCommand : public Command
{
	std::string m_name, m_dir;

public:
	DstOpenSolutionCommand(std::string name, std::string dir)
		: m_name(name), m_dir(dir)
	{
	}
	/**
	 * Executes the command.
	 */
	bool Execute() override;
};
