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
}

Bot::~Bot()
{
	if (m_pCluster)
		delete m_pCluster;
	m_pCluster = nullptr;
}

void Bot::declare_commands()
{
	dpp::slashcommand pingcommand("ping", u8"���� ����ӵ� Ȯ��", m_pCluster->me.id);
	dpp::slashcommand setchannelcommand("set-channel", u8"���� ä�� �Ҵ�", m_pCluster->me.id);
	setchannelcommand.add_option(
		dpp::command_option(dpp::co_channel, "channelid", u8"����� ���� ä���� ����", true)
	);
	dpp::slashcommand setIDcommand("set-user", u8"���� ID ���", m_pCluster->me.id);
	setIDcommand.add_option(
		dpp::command_option(dpp::co_user, "userid", u8"����� ������ ����", true)
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
			Whitelist wl{ dpp::utility::time_f(), channelid, NULL };
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
			Whitelist wl{ dpp::utility::time_f(), NULL, userid };
			auto it = m_whitelist.Insert(event.command.guild_id, wl);
			if (it.second == false) {
				it.first->v.lastTime = dpp::utility::time_f();
				it.first->v.userID.emplace(userid);
			}
			save_whitelist();
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
				//std::cout << "Ÿ�̸� �Լ� ����" << std::endl;
				save_whitelist();
				save_db();

				// �޸� ����

				//std::cout << "Ÿ�̸� �Լ� ����" << std::endl;
				}, 60); // 1�и��� ����

			m_pCluster->on_voice_state_update([this](const dpp::voice_state_update_t& event) {
				//m_m_wl.lock();
				auto guild = m_whitelist.Find(event.state.guild_id);
				if (guild.second) {										// ������� ����̸�
					if (event.state.channel_id) {						// ��ȭ�濡�� ������ �����ϰ� ���� �ߴٸ�
						guild.first->v.channelID.l.lock();
						auto channel = guild.first->v.channelID.s.find(event.state.channel_id);
						if (channel != guild.first->v.channelID.s.end()) {							// ������� ä���̸�
							guild.first->v.userID.l.lock();
							auto user = guild.first->v.userID.s.find(event.state.user_id);
							if (user != guild.first->v.userID.s.end()) {							// ������� ������
								auto tracking_result = m_tracking.Insert(event.state.user_id, DB_DATA(event.state.user_id, event.state.guild_id, dpp::utility::time_f()));
								if (tracking_result.second == true) {
									//std::cout << event.state.user_id << "����\n";
								}
								else {
									//std::cout << "�� ���� �ƴѵ�\n";
								}
							}
							else {
								//std::cout << "��ȭ����� �����ϰ� ������, ������ �������� ����\n";

							}
							guild.first->v.userID.l.unlock();
						}
						else {
							//std::cout << "���� ������ �������ϴ� ��ȭ����\n";
						}
						guild.first->v.channelID.l.unlock();
					}
					else {													// ��ȭ�濡�� �����ٸ�
						auto user = m_tracking.Find(event.state.user_id);
						if (user.second) {									// �������� �������
							user.first->v.END_TIME = dpp::utility::time_f();
							m_DB.Multiple_Insert(user.first->v.userID, user.first->v);
							m_tracking.Remove(event.state.user_id);
							//std::cout << user.first->v.deltaTime() << '\n';
							//std::cout << event.state.user_id << "����\n";
							save_db();
						}
						else {
							//std::cout << "���� ���ϴ� ����\n";
						}
					}
				}
				else {														// ȭ���� �ö�� ���� �ʱ⿡ ȭ��db���Ͽ��� Ȯ��
					if (find_whitelist_in_DB(event.state.guild_id)) {
						//std::cout << "ȭ������ ��� �߰�\n";
						guild = m_whitelist.Find(event.state.guild_id);
						if (event.state.channel_id) {						// ��ȭ�濡�� ������ �����ϰ� ���� �ߴٸ�
							guild.first->v.channelID.l.lock();
							auto channel = guild.first->v.channelID.s.find(event.state.channel_id);
							if (channel != guild.first->v.channelID.s.end()) {							// ������� ä���̸�

								guild.first->v.userID.l.lock();
								auto user = guild.first->v.userID.s.find(event.state.user_id);
								if (user != guild.first->v.userID.s.end()) {							// ������� ������
									auto tracking_result = m_tracking.Insert(event.state.user_id, DB_DATA(event.state.user_id, event.state.guild_id, dpp::utility::time_f()));
									if (tracking_result.second == true) {
										//std::cout << event.state.user_id << "����\n";
									}
									else {
										//std::cout << "�� ���� �ƴѵ�\n";
									}
								}
								else {
									//std::cout << "��ȭ����� �����ϰ� ������, ������ �������� ����\n";
								}
								guild.first->v.userID.l.unlock();
							}
							else {
								//std::cout << "���� ������ �������ϴ� ��ȭ����\n";
							}
							guild.first->v.channelID.l.unlock();
						}
						else {													// ��ȭ�濡�� �����ٸ�
							auto user = m_tracking.Find(event.state.user_id);
							if (user.second) {									// �������� �������
								user.first->v.END_TIME = dpp::utility::time_f();
								m_DB.Multiple_Insert(user.first->v.userID, user.first->v);
								//std::cout << user.first->v.deltaTime() << '\n';
								m_tracking.Remove(event.state.user_id);
								//std::cout << event.state.user_id << "����\n";
								save_db();
							}
							else {
								//std::cout << "���� ���ϴ� ����\n";
							}
						}
					}
					else {
						//std::cout << "ȭ������ ����\n";
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
					auto inserted = wlDB.Insert(serverID, Whitelist(dpp::utility::time_f(), NULL, NULL));
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
		//std::cout << "���������� ȭ�� ���ε�" << "/ " << std::this_thread::get_id() << '\n';
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
	//std::cout << "�ۼ��Ϸ�\n";
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
				auto inserted = m_whitelist.Insert(serverID, Whitelist(dpp::utility::time_f(), NULL, NULL));
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
