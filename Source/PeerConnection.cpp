#include "PeerConnection.h"
#include "talk/examples/peerconnection/client/conductor.h"
#include "talk/examples/peerconnection/client/flagdefs.h"
#include "talk/examples/peerconnection/client/main_wnd.h"
#include "talk/examples/peerconnection/client/peer_connection_client.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/win32socketserver.h"
#include "webrtc/test/field_trial.h"

DWORD WINAPI peer_connection_thread_func(LPVOID lpParameter)
{
	rtc::EnsureWinsockInit();
	rtc::Win32Thread w32_thread;
	rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
    webrtc::test::InitFieldTrialsFromString("");

	rtc::WindowsCommandLineArguments win_args;
	int argc = win_args.argc();
	char **argv = win_args.argv();

	rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
	if (FLAG_help) {
		rtc::FlagList::Print(NULL, false);
		return 0;
	}

	// Abort if the user specifies a port that is outside the allowed
	// range [1, 65535].
	if ((FLAG_port < 1) || (FLAG_port > 65535)) {
		printf("Error: %i is not a valid port.\n", FLAG_port);
		return -1;
	}

	MainWnd wnd(FLAG_server, FLAG_port, FLAG_autoconnect, FLAG_autocall);
	if (!wnd.Create()) {
		ASSERT(false);
		return -1;
	}

	rtc::InitializeSSL();
	PeerConnectionClient client;
	rtc::scoped_refptr<Conductor> conductor(
		new rtc::RefCountedObject<Conductor>(&client, &wnd));

	// Main loop.
	MSG msg;
	BOOL gm;
	while ((gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1) {
		if (!wnd.PreTranslateMessage(&msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	if (conductor->connection_active() || client.is_connected()) {
		while ((conductor->connection_active() || client.is_connected()) &&
			(gm = ::GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1) {
			if (!wnd.PreTranslateMessage(&msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}

	rtc::CleanupSSL();
	return 0;
}