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
