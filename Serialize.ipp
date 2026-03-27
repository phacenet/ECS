


//private helper
template <typename Arg, typename ...Args>
void Serialize::_match_(size_t hash, World& world, std::ifstream& iffile)
{
	std::cout << "Type " << typeid(Arg).name()
		<< " index = " << getTypeIndex<Arg>() << "\n";

	if (getHash<Arg>() == hash)
		world.m_components.at(getTypeIndex<Arg>())->deserialize(iffile);
}


template <typename... Args>
void Serialize::serialize(World& world, const char* filename)
{
	std::ofstream file(filename, std::ios::out | std::ios::binary);
	((world.m_components.at(getTypeIndex<Args>())->serialize(file)), ...);
}

template <typename... Args>
void Serialize::deserialize(World& world, const char* filename)
{
	std::ifstream inFile(filename, std::ios::in | std::ios::binary);
										//^^ implicit flag for ios::in
	
	/*	* Deserialization requires already registered Components
		* _lazy_register_ checks if already registered */
	((world._lazy_register_<Args>()), ...);

	for (size_t i{ 0 }; i < sizeof...(Args); ++i)
	{
		size_t hash;
		if (!inFile.read(reinterpret_cast<char*>(&hash), sizeof(hash)))
			std::cout << "something went wrong\n";

		((_match_<Args>(hash, world, inFile)), ...);
	}
}
