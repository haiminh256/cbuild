#include <string>

struct Config {
	const char* src_path;
};

void defineConfig(Config m_config){
	system("");
}

int main() {
	Config config;
	config.src_path = "src/";
	defineConfig(config);
	return 0;
}