


template <typename... Args>
void Serialize::serialize(World& world, const char* filename)
{
	std::ofstream file(filename);

	
}

template <typename... Args>
void Serialize::deserialize(World& world, const char* filename)
{

}

//user calls world.serialize<Args...>(filename) and world creates a Serialize object, which calls its internal serialize(), which writes the corresponding data to the filestream
//have the basic setup done for world dilineating to Serialize 