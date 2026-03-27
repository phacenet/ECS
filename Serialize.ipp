


template <typename... Args>
void Serialize::serialize(World& world, const char* filename)
{
	std::ofstream file(filename, std::ios::out | std::ios::binary);
	((world.m_components.at(getTypeIndex<Args>())->serialize(file))...);
}

template <typename... Args>
void Serialize::deserialize(World& world, const char* filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
										//^^ implicit flag for ios::in
	
	/*	* Deserialization requires already registered Components
		* _lazy_register_ checks if already registered
	*/
	((world._lazy_register_<Args>())...);

	((world.m_components.at(getTypeIndex<Args>())->deserialize(file))...);
}
