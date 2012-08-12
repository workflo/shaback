--
-- Callbacks
--

_callbacks = {}
_callbacks.preBackup = {}
_callbacks.postBackup = {}

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
