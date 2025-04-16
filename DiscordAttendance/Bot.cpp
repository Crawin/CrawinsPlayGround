#include "stdafx.h"
#include "Bot.h"
std::atomic_int complete = 0;
std::atomic_int fail = 0;
Bot::Bot()
{
	//m_lfum.test();
	//test();
	m_pCluster = new dpp::cluster(m_strBottoken);
}

Bot::~Bot()
{
	if (m_pCluster)
		delete m_pCluster;
	m_pCluster = nullptr;
}

void Bot::declare_commands()
{
	dpp::slashcommand pingcommand("ping", u8"서버 응답속도 확인", m_pCluster->me.id);
	dpp::slashcommand setchannelcommand("set-channel", u8"음성 채널 할당", m_pCluster->me.id);
	setchannelcommand.add_option(
		dpp::command_option(dpp::co_channel, "channelid", u8"기록할 음성 채널을 선택", true)
	);
	dpp::slashcommand setIDcommand("set-user", u8"유저 ID 기록", m_pCluster->me.id);
	setIDcommand.add_option(
		dpp::command_option(dpp::co_user, "userid", u8"기록할 유저를 선택", true)
	);
	m_pCluster->global_bulk_command_create({ pingcommand,setchannelcommand,setIDcommand });
}

void Bot::define_commands()
{
	/* Handle slash command with the most recent addition to D++ features, coroutines! */
	m_pCluster->on_slashcommand([this](const dpp::slashcommand_t& event) {
		auto command = event.command.get_command_name();
		if (command == "ping") {
			event.reply(std::to_string((event.command.get_creation_time() - dpp::utility::time_f()) * 1000) + "ms");
			return;
		}
		if (command == "set-channel") {
			dpp::snowflake channelid = std::get<dpp::snowflake>(event.get_parameter("channelid"));
			auto it = m_umWhitelist.try_emplace(event.command.guild_id, dpp::utility::time_f(), channelid, NULL);
			if (it.second == false) {
				it.first->second.lastTime = dpp::utility::time_f();
				it.first->second.channelID.emplace(channelid);
			}
			event.reply(dpp::message("CHANNEL: " + channelid.str()).set_flags(dpp::m_ephemeral));
			return;
		}
		if (command == "set-user") {
			dpp::snowflake userid = std::get<dpp::snowflake>(event.get_parameter("userid"));
			auto it = m_umWhitelist.try_emplace(event.command.guild_id, dpp::utility::time_f(), NULL, userid);
			if (it.second == false) {
				it.first->second.lastTime = dpp::utility::time_f();
				it.first->second.userID.emplace(userid);
			}
			event.reply(dpp::message("ID: " + userid.str()).set_flags(dpp::m_ephemeral));
			return;
		}
		});
}

void Bot::Run()
{
	/* Output simple log messages to stdout */
	m_pCluster->on_log(dpp::utility::cout_logger());

	m_pCluster->on_ready([this](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {
			declare_commands();

			m_pCluster->start_timer([this](const dpp::timer& timer_handle) {
				//std::cout << "타이머 함수 실행" << std::endl;
				save_whitelist();
				save_db();

				// 메모리 정리
				
				//std::cout << "타이머 함수 종료" << std::endl;
				}, 60); // 1분마다 실행

			m_pCluster->on_voice_state_update([this](const dpp::voice_state_update_t& event) {
				//m_m_wl.lock();
				auto guild = m_umWhitelist.find(event.state.guild_id);
				if (guild != m_umWhitelist.end()) {		// 기록중인 길드이면
					if (event.state.channel_id) {		// 통화방에서 나간걸 제외하고 뭔갈 했다면
						auto channel = guild->second.channelID.find(event.state.channel_id);
						if (channel != guild->second.channelID.end()) {	// 기록중인 채널이면
							auto user = guild->second.userID.find(event.state.user_id);
							if (user != guild->second.userID.end()) {	// 기록중인 유저면
								m_umTracking.try_emplace(event.state.user_id, dpp::utility::time_f());	// 추적 시작
								//std::cout << event.state.user_id << "입장\n";
							}
						}
					}
					else {								// 통화방에서 나갔다면
						auto user = m_umTracking.find(event.state.user_id);
						if (user != m_umTracking.end()) {	// 추적하던 유저라면
							user->second.END_TIME = dpp::utility::time_f();
							//std::cout << user->second.END_TIME - user->second.START_TIME << "s" << std::endl;
							m_qDBdata.emplace(user);
							m_umTracking.erase(event.state.user_id);
							//std::cout << event.state.user_id << "퇴장\n";
						}
					}
				}
				else {			// 화리에 올라와 있지 않기에 화리db파일에서 확인
					//m_m_wl.unlock();
					if (find_whitelist_in_DB(event.state.guild_id)) {
						//std::cout << "화리에서 발견\n";
						guild = m_umWhitelist.find(event.state.guild_id);
						if (event.state.channel_id) {		// 통화방에서 나간걸 제외하고 뭔갈 했다면
							auto channel = guild->second.channelID.find(event.state.channel_id);
							if (channel != guild->second.channelID.end()) {	// 기록중인 채널이면
								auto user = guild->second.userID.find(event.state.user_id);
								if (user != guild->second.userID.end()) {	// 기록중인 유저면
									m_umTracking.try_emplace(event.state.user_id, dpp::utility::time_f());	// 추적 시작
									std::cout << event.state.user_id << "입장\n";
								}
							}
						}
						else {								// 통화방에서 나갔다면
							auto user = m_umTracking.find(event.state.user_id);
							if (user != m_umTracking.end()) {	// 추적하던 유저라면
								user->second.END_TIME = dpp::utility::time_f();
								//std::cout << user->second.END_TIME - user->second.START_TIME << "s" << std::endl;
								m_qDBdata.emplace(user);
								m_umTracking.erase(event.state.user_id);
								std::cout << event.state.user_id << "퇴장\n";
							}
						}
					}
					else {
						//std::cout << "화리에도 없음\n";
					}

				}
				});
		}
		});

	define_commands();
	/* Start the bot */
	m_pCluster->start(dpp::st_wait);
}

void Bot::save_db()
{
	std::ifstream inputFile("DB.txt");
	if (!inputFile.is_open()) {
		std::cerr << "파일을 읽을 수 없습니다!" << std::endl;
	}
	std::ofstream outputFile("DB.txt");
	//while (m_qUserDB.size()) {
	//	auto data = m_qUserDB.front();
	//	m_qUserDB.pop();
	//	if (data.END_TIME == 0) {

	//	}
	//	else {
	//		auto delta_time = data.END_TIME - data.START_TIME;
	//	}
	//}
//	if (!outputFile.is_open()) {
//		std::cerr << "파일을 열 수 없습니다!" << std::endl;
//		return;
//	}
//	// XML 형식으로 출력
//	std::string content = R"(<[server_ID]>
//	<CURRENT>
//	</CURRENT>
//	<USERS>
//	</USERS>
//</[server_ID]>
//)";
//	outputFile << content;
	// 파일 닫기
	outputFile.close();
	return;
}

void Bot::save_whitelist()
{
	std::unordered_map<dpp::snowflake, Whitelist> wlDB;
	m_sm_wldb.lock_shared();
	std::ifstream inputFile("WhiteList.json",std::ios::binary);
	std::string line;
	if (inputFile) {
		// load_untracking_whitelist
		while (std::getline(inputFile, line)) {
			if (line.find("\"SERVERS\": [") != std::string::npos) {
				unsigned long long serverid;
				bool onmemory = true;
				while (std::getline(inputFile, line)) {
					if (line[0] == ']')
						break;

					if (line.find("\"SERVERID\":") != std::string::npos) {
						size_t start = line.find(" ");
						size_t end = line.find(",");
						serverid = std::stoull(line.substr(start + 1, end - (start + 1)));
						auto result_it = m_umWhitelist.find(serverid);
						if (result_it == m_umWhitelist.end()) {
							//메모리에 올라가있지 않은 화리
							wlDB.try_emplace(serverid);
							onmemory = false;
						}
						else {
							onmemory = true;
						}
						continue;
					}

					if (onmemory == true) {
						continue;
					}

					if (line.find("\"CHANNELS\":") != std::string::npos) {
						size_t start = line.find("[");
						size_t end = line.find("]");
						size_t curr = start;
						std::string num;
						while (curr < end) {
							if (line[curr] == ',') {
								num = line.substr(start + 1, curr - (start + 1));
								wlDB[serverid].channelID.emplace(std::stoull(num));
								start = curr;
							}
							++curr;
						}
						num = line.substr(start + 1, end - (start + 1));
						if (num.size()) {
							wlDB[serverid].channelID.emplace(std::stoull(num));
						}
						continue;
					}
					if (line.find("\"USERS\":") != std::string::npos) {
						size_t start = line.find("[");
						size_t end = line.find("]");
						size_t curr = start;
						std::string num;
						while (curr < end) {
							if (line[curr] == ',') {
								num = line.substr(start + 1, curr - (start + 1));
								wlDB[serverid].userID.emplace(std::stoull(num));
								start = curr;
							}
							++curr;
						}
							num = line.substr(start + 1, end - (start + 1));
						if (num.size()) {
							wlDB[serverid].userID.emplace(std::stoull(num));
						}
						continue;
					}
				}
			}
		}
		inputFile.close();
	}
	else
	{
		std::cerr << "파일을 읽을 수 없습니다!" << std::endl;
	}
	m_sm_wldb.unlock_shared();

	m_sm_wldb.lock();
	std::ofstream outputFile("WhiteList.json",std::ios::binary);
	if (outputFile) {
		outputFile << "{\n\"SERVERS\": [\n";
		bool firstServer = true;
		for (const auto& [serverid, wl] : m_umWhitelist) {
			if (!firstServer)
				outputFile << ",\n";
			else
				firstServer = false;
			outputFile << "{\n\"SERVERID\": " << serverid << ",\n\"CHANNELS\": [";

			bool firstChannel = true;
			for (const auto& ch : wl.channelID) {
				if (!firstChannel)
					outputFile << ", ";
				else
					firstChannel = false;
				outputFile << ch;
			}

			outputFile << "],\n\"USERS\": [";
			firstChannel = true;
			for (const auto& usr : wl.userID) {
				if (!firstChannel)
					outputFile << ", ";
				else
					firstChannel = false;
				outputFile << usr;
			}
			outputFile << "]\n}";
		}

		for (const auto& [serverid, wl] : wlDB) {
			if (!firstServer)
				outputFile << ",\n";
			else
				firstServer = false;
			outputFile << "{\n\"SERVERID\": " << serverid << ",\n\"CHANNELS\": [";

			bool firstChannel = true;
			for (const auto& ch : wl.channelID) {
				if (!firstChannel)
					outputFile << ", ";
				else
					firstChannel = false;
				outputFile << ch;
			}

			outputFile << "],\n\"USERS\": [";
			firstChannel = true;
			for (const auto& usr : wl.userID) {
				if (!firstChannel)
					outputFile << ", ";
				else
					firstChannel = false;
				outputFile << usr;
			}
			outputFile << "]\n}";
		}

		outputFile << "\n]\n}";
	}
	std::cout << "작성완료\n";
	m_sm_wldb.unlock();
}

bool Bot::find_whitelist_in_DB(const unsigned long long& guild_id)
{
	std::string line;
	bool found_on_db = false;
	m_sm_wldb.lock_shared();
	std::ifstream inputFile("WhiteList.json", std::ios::binary);
	if (inputFile) {
		while (std::getline(inputFile, line)) {
			if (line.find("\"SERVERS\": [") != std::string::npos) {
				unsigned long long serverid;
				while (std::getline(inputFile, line)) {
					if (line[0] == ']')
						break;

					if (line.find("\"SERVERID\":") != std::string::npos) {
						size_t start = line.find(" ");
						size_t end = line.find(",");
						serverid = std::stoull(line.substr(start + 1, end - (start + 1)));
						if (guild_id == serverid) {
							//m_m_wl.lock();
							m_umWhitelist.try_emplace(serverid, dpp::utility::time_f(), NULL, NULL);
							//m_m_wl.unlock();
							found_on_db = true;
						}
						else {
							found_on_db = false;
						}
						continue;
					}
					if (found_on_db == false) {
						continue;
					}

					if (line.find("\"CHANNELS\":") != std::string::npos) {
						size_t start = line.find("[");
						size_t end = line.find("]");
						size_t curr = start;
						std::string num;
						//m_m_wl.lock();
						while (curr < end) {
							if (line[curr] == ',') {
								num = line.substr(start + 1, curr - (start + 1));
								m_umWhitelist[serverid].channelID.emplace(std::stoull(num));
								start = curr;
							}
							++curr;
						}
						num = line.substr(start + 1, end - (start + 1));
						if (num.size()) {
							m_umWhitelist[serverid].channelID.emplace(std::stoull(num));
						}
						//m_m_wl.unlock();
						continue;
					}
					if (line.find("\"USERS\":") != std::string::npos) {
						size_t start = line.find("[");
						size_t end = line.find("]");
						size_t curr = start;
						std::string num;
						while (curr < end) {
							if (line[curr] == ',') {
								num = line.substr(start + 1, curr - (start + 1));
								m_umWhitelist[serverid].userID.emplace(std::stoull(num));
								start = curr;
							}
							++curr;
						}
						num = line.substr(start + 1, end - (start + 1));
						if (num.size()) {
							m_umWhitelist[serverid].userID.emplace(std::stoull(num));
						}
						break;
					}
				}
			}
		}
		inputFile.close();
	}
	else
	{
		std::cerr << "파일을 읽을 수 없습니다!" << std::endl;
	}
	m_sm_wldb.unlock_shared();
	return found_on_db;
}

void Bot::cleanup_Memory()
{
}

void Bot::test()
{
	std::vector<std::thread> vt;

	for (int i = 0; i < 5; ++i) {
		vt.emplace_back(&Bot::test_func, this, i * 100000 + 1);
	}

	for (auto& t : vt) {
		t.join();
	}
	m_lfhsl.Print();
	std::cout << "finish";
}

void Bot::test_func(const int& start)
{
	for (volatile int i = 0; i < 100000; ++i) {
		//printf("Threadid: %d [%d]\n", m_lfmsl.m_thread_id, i);
		//std::cout << m_lfmsl.m_thread_id << std::endl;
		if (m_lfhsl.Insert(i + start, Whitelist(i + start, i + start, i + start)) == false) {
			std::cout << i + start << "삽입 실패\n";
		}
		if (m_lfhsl.Find(i + start) == false) {
			std::cout << i + start << "검색 실패\n";
		}
		if (m_lfhsl.Remove(i + start) == false) {
			std::cout << i + start << "삭제 실패\n";
		}
	}
	//m_lfhsl.m_free_queue
}
