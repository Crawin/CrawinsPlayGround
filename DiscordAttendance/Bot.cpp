#include "stdafx.h"
#include "Bot.h"
std::atomic_int complete = 0;
std::atomic_int fail = 0;
Bot::Bot()
{
	//test(1);
	//save_whitelist();
	//test(1);
	m_pCluster = new dpp::cluster(m_strBottoken);
	m_dBeginTime = dpp::utility::time_f();
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
	dpp::slashcommand runningtimecommand("runningtime", u8"봇 가동 시간 확인", m_pCluster->me.id);
	dpp::slashcommand setchannelcommand("set-channel", u8"음성 채널 할당", m_pCluster->me.id);
	setchannelcommand.add_option(
		dpp::command_option(dpp::co_channel, "channelid", u8"기록할 음성 채널을 선택", true)
	);
	dpp::slashcommand setIDcommand("set-user", u8"유저 ID 기록", m_pCluster->me.id);
	setIDcommand.add_option(
		dpp::command_option(dpp::co_user, "userid", u8"기록할 유저를 선택", true)
	);
	m_pCluster->global_bulk_command_create({ pingcommand,runningtimecommand,setchannelcommand,setIDcommand });
}

void Bot::define_commands()
{
	/* Handle slash command with the most recent addition to D++ features, coroutines! */
	m_pCluster->on_slashcommand([this](const dpp::slashcommand_t& event) {
		auto command = event.command.get_command_name();
		if (command == "ping") {
			event.reply(std::to_string((dpp::utility::time_f() - event.command.get_creation_time()) * 1000) + "ms");
			return;
		}
		if (command == "set-channel") {
			dpp::snowflake channelid = std::get<dpp::snowflake>(event.get_parameter("channelid"));
			Whitelist wl{ dpp::utility::time_f(), channelid, 0 };
			auto it = m_whitelist.Insert(event.command.guild_id, wl);
			if (it.second == false) {
				it.first->v.lastTime = dpp::utility::time_f();
				it.first->v.channelID.emplace(channelid);
			}
			save_whitelist();
			event.reply(dpp::message("CHANNEL: " + channelid.str()).set_flags(dpp::m_ephemeral));
			return;
		}
		if (command == "set-user") {
			dpp::snowflake userid = std::get<dpp::snowflake>(event.get_parameter("userid"));
			Whitelist wl{ dpp::utility::time_f(), 0, userid };
			auto it = m_whitelist.Insert(event.command.guild_id, wl);
			if (it.second == false) {
				it.first->v.lastTime = dpp::utility::time_f();
				it.first->v.userID.emplace(userid);
			}
			save_whitelist();
			event.reply(dpp::message("ID: " + userid.str()).set_flags(dpp::m_ephemeral));
			return;
		}
		if (command == "runningtime") {
			double deltaTime = dpp::utility::time_f() - m_dBeginTime;
			int days = static_cast<int>(deltaTime / 86400);
			int hours = static_cast<int>((deltaTime - days * 86400) / 3600);
			int minutes = static_cast<int>((deltaTime - days * 86400 - hours * 3600) / 60);
			int seconds = static_cast<int>(deltaTime) % 60;

			std::string runningtime = "";
			if (days != 0)
				runningtime += std::to_string(days) + " days ";
			if (hours != 0)
				runningtime += std::to_string(hours) + " hours ";
			if (minutes != 0)
				runningtime += std::to_string(minutes) + " mins ";
			if (seconds != 0)
				runningtime += std::to_string(seconds) + " secs ";

			if (runningtime.empty()) {
				runningtime += "0 sec";
			}

			event.reply(runningtime);
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
				auto guild = m_whitelist.Find(event.state.guild_id);
				if (guild.second) {										// 기록중인 길드이면
					if (event.state.channel_id) {						// 통화방에서 나간걸 제외하고 뭔갈 했다면
						guild.first->v.channelID.l.lock();
						auto channel = guild.first->v.channelID.s.find(event.state.channel_id);
						if (channel != guild.first->v.channelID.s.end()) {							// 기록중인 채널이면
							guild.first->v.userID.l.lock();
							auto user = guild.first->v.userID.s.find(event.state.user_id);
							if (user != guild.first->v.userID.s.end()) {							// 기록중인 유저면
								auto tracking_result = m_tracking.Insert(event.state.user_id, DB_DATA(event.state.user_id, event.state.guild_id, dpp::utility::time_f()));
								if (tracking_result.second == true) {
									//std::cout << event.state.user_id << "입장\n";
								}
								else {
									//std::cout << "뭐 별일 아닌듯\n";
								}
							}
							else {
								//std::cout << "통화방까진 추적하고 있지만, 유저를 추적하지 않음\n";

							}
							guild.first->v.userID.l.unlock();
						}
						else {
							//std::cout << "길드는 같지만 추적안하는 통화방임\n";
						}
						guild.first->v.channelID.l.unlock();
					}
					else {													// 통화방에서 나갔다면
						auto user = m_tracking.Find(event.state.user_id);
						if (user.second) {									// 추적중인 유저라면
							user.first->v.END_TIME = dpp::utility::time_f();
							m_DB.Multiple_Insert(user.first->v.userID, user.first->v);
							m_tracking.Remove(event.state.user_id);
							//std::cout << user.first->v.deltaTime() << '\n';
							//std::cout << event.state.user_id << "퇴장\n";
							save_db();
						}
						else {
							//std::cout << "추적 안하는 애임\n";
						}
					}
				}
				else {														// 화리에 올라와 있지 않기에 화리db파일에서 확인
					if (find_whitelist_in_DB(event.state.guild_id)) {
						//std::cout << "화리에서 길드 발견\n";
						guild = m_whitelist.Find(event.state.guild_id);
						if (event.state.channel_id) {						// 통화방에서 나간걸 제외하고 뭔갈 했다면
							guild.first->v.channelID.l.lock();
							auto channel = guild.first->v.channelID.s.find(event.state.channel_id);
							if (channel != guild.first->v.channelID.s.end()) {							// 기록중인 채널이면

								guild.first->v.userID.l.lock();
								auto user = guild.first->v.userID.s.find(event.state.user_id);
								if (user != guild.first->v.userID.s.end()) {							// 기록중인 유저면
									auto tracking_result = m_tracking.Insert(event.state.user_id, DB_DATA(event.state.user_id, event.state.guild_id, dpp::utility::time_f()));
									if (tracking_result.second == true) {
										//std::cout << event.state.user_id << "입장\n";
									}
									else {
										//std::cout << "뭐 별일 아닌듯\n";
									}
								}
								else {
									//std::cout << "통화방까진 추적하고 있지만, 유저를 추적하지 않음\n";
								}
								guild.first->v.userID.l.unlock();
							}
							else {
								//std::cout << "길드는 같지만 추적안하는 통화방임\n";
							}
							guild.first->v.channelID.l.unlock();
						}
						else {													// 통화방에서 나갔다면
							auto user = m_tracking.Find(event.state.user_id);
							if (user.second) {									// 추적중인 유저라면
								user.first->v.END_TIME = dpp::utility::time_f();
								m_DB.Multiple_Insert(user.first->v.userID, user.first->v);
								//std::cout << user.first->v.deltaTime() << '\n';
								m_tracking.Remove(event.state.user_id);
								//std::cout << event.state.user_id << "퇴장\n";
								save_db();
							}
							else {
								//std::cout << "추적 안하는 애임\n";
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
	std::ifstream inputFile("DB.json", std::ios::binary);
	nlohmann::ordered_json j;
	if (inputFile) {
		inputFile >> j;
		inputFile.close();
	}

	m_DB.Save_DB(j);

	std::ofstream outputFile("DB.json", std::ios::binary);
	if (outputFile) {
		outputFile << j.dump(4);
		outputFile.close();
	}
	return;
}

void Bot::save_whitelist()
{
	m_sm_wldb.lock();
	std::ifstream inputFile("WhiteList.json", std::ios::binary);
	crawin::LF_hash_skiplist<dpp::snowflake, Whitelist> wlDB;
	if (inputFile) {
		nlohmann::json j;
		inputFile >> j;

		if (j.contains("SERVERS")) {
			for (const auto& server : j["SERVERS"]) {
				unsigned long long serverID = std::stoull(server["SERVERID"].get<std::string>());
				auto result = m_whitelist.Find(serverID);
				if (result.second == false) {
					auto inserted = wlDB.Insert(serverID, Whitelist(dpp::utility::time_f(), 0, 0));
					if (server.contains("CHANNELS")) {
						for (const auto& ch : server["CHANNELS"]) {
							unsigned long long channelID = std::stoull(ch.get<std::string>());
							inserted.first->v.lastTime = dpp::utility::time_f();
							inserted.first->v.channelID.emplace(channelID);
						}
					}
					if (server.contains("USERS")) {
						for (const auto& user : server["USERS"]) {
							unsigned long long userID = std::stoull(user.get<std::string>());
							inserted.first->v.lastTime = dpp::utility::time_f();
							inserted.first->v.userID.emplace(userID);
						}
					}
				}
			}
		}
		//std::cout << "미추적중인 화리 업로드" << "/ " << std::this_thread::get_id() << '\n';
		inputFile.close();
	}

	std::ofstream outputFile("WhiteList.json",std::ios::binary);
	if (outputFile) {
		nlohmann::ordered_json j;
		m_whitelist.Save(j);
		wlDB.Save(j);
		outputFile << j.dump(4);
		outputFile.close();
	}
	//std::cout << "작성완료\n";
	m_sm_wldb.unlock();
}

bool Bot::find_whitelist_in_DB(const unsigned long long& guild_id)
{
	bool found_on_db = false;
	m_sm_wldb.lock_shared();
	std::ifstream inputFile("WhiteList.json", std::ios::binary);
	if (inputFile) {
		nlohmann::json j;
		inputFile >> j;
		for (const auto& server : j["SERVERS"]) {
			unsigned long long serverID = std::stoull(server["SERVERID"].get<std::string>());
			if (serverID == guild_id) {
				auto inserted = m_whitelist.Insert(serverID, Whitelist(dpp::utility::time_f(), 0, 0));
				if (inserted.second) {
					if (server.contains("CHANNELS")) {
						for (const auto& ch : server["CHANNELS"]) {
							unsigned long long channelID = std::stoull(ch.get<std::string>());
							inserted.first->v.lastTime = dpp::utility::time_f();
							inserted.first->v.channelID.emplace(channelID);
						}
					}
					if (server.contains("USERS")) {
						for (const auto& user : server["USERS"]) {
							unsigned long long userID = std::stoull(user.get<std::string>());
							inserted.first->v.lastTime = dpp::utility::time_f();
							inserted.first->v.userID.emplace(userID);
						}
					}
					found_on_db = true;
				}
			}
		}
		inputFile.close();
	}
	m_sm_wldb.unlock_shared();
	return found_on_db;
}

void Bot::cleanup_Memory()
{
}

void Bot::test(int n)
{
	std::vector<std::thread> vt;
	while (n--) {
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < 1; ++i) {
			vt.emplace_back(&Bot::test_func, this, i * 10000000 + 1);
		}
		for (auto& t : vt) {
			t.join();
		}
		vt.clear();
		//m_whitelist.Clear();
		////std::cout << "finish\n";
		////std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() <<"ms" << std::endl;
		//m_whitelist.Print();
	}
}

void Bot::test_func(const int& start)
{
	auto result = m_whitelist.Insert(99, Whitelist(0, 88, 77));
	//m_whitelist.Remove(99);
	result.first->v.channelID.emplace(66);
	result.first->v.userID.emplace(55);

	result = m_whitelist.Insert(88, Whitelist(0, 44, 33));
	//result = m_whitelist.Multiple_Insert(88, Whitelist(0, 22, 11));
	//m_whitelist.Print();
	//result = m_whitelist.Insert(99, Whitelist(0, 66, 55));
	////std::cout << result.second << '\n';
}
