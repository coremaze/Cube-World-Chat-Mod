#include "CWSDK/cwsdk.h"

class ChatMod : GenericMod {
	virtual void Initialize() override {
		// Very low priority because we don't want to interfere with any other mods, since we'll be canceling
		// almost every chat message.
		OnChatPriority = VeryLowPriority;
	}
	virtual int OnChat(std::wstring* message) override {
		// Make sure the vanilla commands don't get eaten 
		for (const std::wstring& str : { std::wstring(L"/pet"), std::wstring(L"/sit"), std::wstring(L"/dance"), std::wstring(L"/namepet"), std::wstring(L"/check"), std::wstring(L"/wave") }) {
			if (message->size() >= str.size() && 
				!message->compare(0, str.length(), str)) {
				return 0;
			}
		}

		// Send a packet instead of just writing to chat, if you are in an online session
		// An online session shall be defined as whether you're connected to someone else's steam ID, or if your host has more than 1 connection
		cube::Game* game = cube::GetGame();
		if (cube::SteamUser()->GetSteamID() != game->client.host_steam_id || game->host.connections.size() > 1) {
			// Construct a chat packet
			// Thanks to Andoryuuta for reverse engineering these packets, even before the beta was released
			BytesIO bytesio;

			bytesio.Write<u32>(0x0E); //Packet ID
			bytesio.Write<u32>(message->size()); //Message size
			for (int i = 0; i < message->size(); i++) bytesio.Write<wchar_t>(message->c_str()[i]); //Message

			cube::SteamNetworking()->SendP2PPacket(game->client.host_steam_id, bytesio.Data(), bytesio.Size(), k_EP2PSendReliable, 1);

			return 1;
		}
		return 0;
	}
};

EXPORT ChatMod* MakeMod() {
	return new ChatMod();
}