


//private helper
template <typename Arg>
void Serialize::_match_(size_t hash, World& world, std::ifstream& iffile)
{
	std::cout << "checking " << typeid(Arg).name() << " hash: " << getHash<Arg>() << " vs file hash: " << hash << "\n";
	if (getHash<Arg>() == hash)
		world.m_components.at(getTypeIndex<Arg>())->deserialize(iffile);
}


template <typename... Args>
void Serialize::serialize(World& world, const char* filename)
{
	std::ofstream file(filename, std::ios::out | std::ios::binary);

	if (!file.is_open())
		throw std::logic_error("Failed to create/open file for serialization");

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

	if (!inFile.is_open())
		throw std::logic_error("Failed to open file for deserialization");

	for (size_t i{ 0 }; i < sizeof...(Args); ++i)
	{
		size_t hash;
		if (!inFile.read(reinterpret_cast<char*>(&hash), sizeof(hash)))
			throw std::logic_error("Failed to read passed file's hash");

		((_match_<Args>(hash, world, inFile)), ...);
	}
	_create_entities_<Args...>(world);
}

//private
template <typename... Args>
void Serialize::_create_entities_(World& world)
{
	std::unordered_set<uint32_t> setIDs;

	((setIDs.insert((world.m_components.at(getTypeIndex<Args>())->getDense()).begin(),
					(world.m_components.at(getTypeIndex<Args>())->getDense()).end())), ...);

	if (setIDs.empty())
		return;

	size_t highestID = *(std::max_element(setIDs.begin(), setIDs.end()));

	for (auto& e : setIDs)
		std::cout << e << ", ";

	//creating [0, highestID]
	for (uint32_t i{ 0 }; i <= highestID; ++i)
	{
		if (setIDs.contains(i))
			world._register_aliveID_();
		else
			world._register_freeID_();
	}
}
