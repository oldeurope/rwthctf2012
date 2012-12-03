#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

struct IdAndKey {
	uint64_t id;
	unsigned char key[32];
};

IdAndKey getIDAndKey(const char* ip) {
	using namespace std;

	IdAndKey result;
	fstream file(ip, ios::in | ios::binary);

	if(file) {
		file.read((char*)&result.id,  8);
		file.read((char*)result.key, 32);

		if(file.fail()) {
			fprintf(stderr, "Broken key file: %s!\n", ip);
			exit(3);
		}

		file.close();
	} else {
		fprintf(stderr, "Could not open key file %s!\n", ip);
		exit(3);
	}

	return result;
}

void setIDAndKey(const char* ip, const IdAndKey& data) {
	using namespace std;

	fstream file(ip, ios::out | ios::binary);

	if(file) {
		file.write((const char*)&data.id,  8);
		file.write((const char*)data.key, 32);

		if(file.fail()) {
			fprintf(stderr, "Could not write to key file %s!\n", ip);
			exit(3);
		}

		file.close();
	} else {
		fprintf(stderr, "Could not open key file %s!\n", ip);
		exit(3);
	}
}
