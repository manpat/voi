template<class GenType>
void AudioManager::RegisterAudioGeneratorType(const std::string& genname){
	// If it exists, go nuts
	if(findin(audioGeneratorTemplates, genname)) {
		throw "Trying to register duplicated AudioGeneratorType " + genname;
	}

	audioGeneratorTemplates[genname] = std::make_shared<AudioGeneratorFactory<GenType>>();
}
