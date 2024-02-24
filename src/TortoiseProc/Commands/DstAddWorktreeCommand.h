#pragma once
#include "Command.h"

/**
 * \ingroup TortoiseProc
 * Shows the DST Add Worktree dialog.
 */
class DstAddWorktreeCommand : public Command
{
public:
	/**
	 * Executes the command.
	 */
	bool Execute() override;
};
