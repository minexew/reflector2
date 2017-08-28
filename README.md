reflector 2
===========

Main usages
-----------

### Generating protocol [de]serialization code

- network protocol (special-purpose packets)
- inter-thread (more "natural" entities) e.g. between ZoneInstances

Example:
Player1 in zone1 whispers player2 in zone2
ZoneInstance sends CmdWhisper to Realm
Realm finds player2, sends CmdWhisper to zone2
zone2 processess CmdWhisper, player2 receives the message

### Generating reflection for component configuration

class LoginServer {
    configure_this
    std::vector<RealmConnectionInfo> realms;

    configure_this
    void setServerName(std::string&& name);
}

### Generating script bindings for native classes

TODO
----

Build a class-based semantic model in Python
Allow type search etc.
