class Signals(object):
    def __init__(self):
        self._signals = {}

    def _connect(self, obj, name, handler, connector):
        ret = self._signals.setdefault(obj, {})

        hid = connector(name, handler)
        ret.setdefault(name, []).append(hid)

        return hid

    def connect_signal(self, obj, name, handler):
        return self._connect(obj, name, handler, obj.connect)

    def connect_signal_after(self, obj, name, handler):
        return self._connect(obj, name, handler, obj.connect_after)

    def disconnect_signals(self, obj):
        if obj not in self._signals:
            return False

        for name in self._signals[obj]:
            for hid in self._signals[obj][name]:
                obj.disconnect(hid)

        del self._signals[obj]
        return True

    def block_signal(self, obj, name):
        if obj not in self._signals:
            return False

        if name not in self._signals[obj]:
            return False

        for hid in self._signals[obj][name]:
            obj.handler_block(hid)

        return True

    def unblock_signal(self, obj, name):
        if obj not in self._signals:
            return False

        if name not in self._signals[obj]:
            return False

        for hid in self._signals[obj][name]:
            obj.handler_unblock(hid)

        return True

    def disconnect_signal(self, obj, name):
        if obj not in self._signals:
            return False

        if name not in self._signals[obj]:
            return False

        for hid in self._signals[obj][name]:
            obj.disconnect(hid)

        del self._signals[obj][name]

        if len(self._signals[obj]) == 0:
            del self._signals[obj]

        return True