#include "DatabaseDownloader.h"

DatabaseDownloader::DatabaseDownloader()
{
}

DatabaseDownloader::~DatabaseDownloader()
{
}

bool DatabaseDownloader::check_and_download_database(const string& repo_owner, const string& repo_name, const string& asset_name, const string& local_file_path, const string& input_db_file_path, const string& output_db_file_path, const bool& reextract_db)
{
	// Step 1: Send an HTTP GET request to the API endpoint for the repository's releases
	string releases_url = "https://api.github.com/repos/" + repo_owner + "/" + repo_name + "/releases";
	auto releases_response = cpr::Get(cpr::Url{ releases_url });

	if (releases_response.status_code != 200)
	{
		return false;
	}

	// Step 2: Parse the JSON response to get the most recent release and its assets
	auto releases = json::parse(releases_response.text);
	if (releases.empty())
	{
		return false;
	}
	auto latest_release = releases[0];

	// Step 3: Check the "published_at" timestamp of the latest release
	string release_published_at = latest_release["published_at"].get<string>();

	// Convert the release's published_at timestamp to std::time_t
	tm release_tm = {};
	istringstream release_time_stream(release_published_at);
	release_time_stream >> get_time(&release_tm, "%Y-%m-%dT%H:%M:%SZ");
	time_t release_timestamp = mktime(&release_tm);

	// Generate the local file name based on the release date
	std::ostringstream local_file_name_stream;
	local_file_name_stream << std::put_time(&release_tm, "%Y-%m-%d") << ".zip";
	string local_file_name = local_file_name_stream.str();
	string local_file_with_path;
	if (local_file_path.empty())
	{
		local_file_with_path = local_file_name;
	}
	else
	{
		local_file_with_path = local_file_path + "/" + local_file_name;
	}

	// Step 4: Compare the release dates to decide whether to download the new release or not
	if (filesystem::exists(local_file_with_path))
	{
		if (reextract_db)
		{
			return extract_database(local_file_with_path, input_db_file_path, output_db_file_path);
		}
		else
		{
			return true;
		}
	}

	// Step 5: If the asset has been updated more recently, download and replace the local file
	json target_asset;
	for (const auto& asset : latest_release["assets"])
	{
		if (asset["name"].get<string>() == asset_name)
		{
			target_asset = asset;
			break;
		}
	}

	if (target_asset.is_null())
	{
		return false;
	}
	auto download_url = target_asset["browser_download_url"].get<string>();
	cpr::Response download_response = cpr::Get(cpr::Url{ download_url },
		cpr::ProgressCallback([&](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow, cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow, intptr_t userdata) -> bool
			{
				if (downloadTotal > 0) {
					double progress = static_cast<double>(downloadNow) / downloadTotal * 100;
					cout << "\rDownloaded " << downloadNow << " / " << downloadTotal << " bytes. ("
						<< fixed << std::setprecision(2) << progress << "%)" << flush;
				}
				return true;
			}));

	if (download_response.status_code != 200)
	{
		return false;
	}

	// Step 6: Write the downloaded data to the local file
	ofstream local_file(local_file_with_path, ios::binary);
	local_file.write(download_response.text.data(), download_response.text.size());
	local_file.close();

	cout << endl;

	return extract_database(local_file_with_path, input_db_file_path, output_db_file_path);
}

bool DatabaseDownloader::extract_database(const string& zip_file_path, const string& input_file_path, const string& output_file_path) {
	cout << "Extracting database..." << endl;

	mz_zip_archive zip_archive;
	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, zip_file_path.c_str(), 0)) {
		// Handle the error
		return false;
	}

	// Replace "subfolder/database_file_name.ext" with the correct path within the .zip file
	int file_index = mz_zip_reader_locate_file(&zip_archive, input_file_path.c_str(), nullptr, 0);
	if (file_index < 0) {
		// Handle the error
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	mz_zip_archive_file_stat file_stat;
	if (!mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat)) {
		// Handle the error
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	vector<unsigned char> extracted_data;
	extracted_data.resize(file_stat.m_uncomp_size);

	if (!mz_zip_reader_extract_to_mem(&zip_archive, file_index, extracted_data.data(), extracted_data.size(), 0)) {
		// Handle the error
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	mz_zip_reader_end(&zip_archive);

	std::ofstream output_file(output_file_path, ios::binary);
	output_file.write(reinterpret_cast<const char*>(extracted_data.data()), extracted_data.size());
	output_file.close();

	cout << "Database extracted." << endl;

	return true;
}
