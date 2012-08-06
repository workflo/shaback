--
-- Callbacks
--

_callbacks = {}
_callbacks.preBackup = {}

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
