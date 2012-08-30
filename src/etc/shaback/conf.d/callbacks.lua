--
-- Callbacks
--

_callbacks = {}
_callbacks.preBackup = {}
_callbacks.postBackup = {}
_callbacks.enterDir = {}
_callbacks.leaveDir = {}


--
-- Registers a new pre-backup callback.
--
function addPreBackupCallback(cb)
    _callbacks.preBackup[#_callbacks.preBackup +1] = cb
end


--
-- Runs all registered pre-backup callbacks.
--
function _runPreBackupCallbacks()
    for i,v in ipairs(_callbacks.preBackup) do
        v()
    end
end


--
-- Registers a new post-backup callback.
--
function addPostBackupCallback(cb)
    _callbacks.postBackup[#_callbacks.postBackup +1] = cb
end


--
-- Runs all registered post-backup callbacks.
--
function _runPostBackupCallbacks(numFilesRead, numBytesRead, numFilesStored, numBytesStored, numErrors)
    for i,v in ipairs(_callbacks.postBackup) do
        v(numFilesRead, numBytesRead, numFilesStored, numBytesStored, numErrors)
    end
end


--
-- Registers a new enter-directory callback.
--
function addEnterDirCallback(cb)
    _callbacks.enterDir[#_callbacks.enterDir +1] = cb
end


--
-- Runs all registered enter-directory callbacks.
--
function _runEnterDirCallbacks(dir)
    for i,v in ipairs(_callbacks.enterDir) do
        v(dir)
    end
end


--
-- Registers a new leave-directory callback.
--
function addLeaveDirCallback(cb)
    _callbacks.leaveDir[#_callbacks.leaveDir +1] = cb
end


--
-- Runs all registered leave-directory callbacks.
--
function _runLeaveDirCallbacks(dir)
    for i,v in ipairs(_callbacks.leaveDir) do
        v(dir)
    end
end
