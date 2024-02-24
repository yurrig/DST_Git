#pragma once
#include "Command.h"

/**
 * \ingroup TortoiseProc
 * Shows the DST Remove Worktree dialog.
 */
class DstDropWorktreeCommand : public Command
{
public:
	/**
	 * Executes the command.
	 */
	bool Execute() override;
};
