#pragma once

#include <ydb/public/lib/ydb_cli/common/profile_manager.h>
#include <ydb/public/lib/ydb_cli/common/root.h>

namespace NYdb {
namespace NConsoleClient {

struct TClientSettings {
    // Whether to use secure connection or not
    TMaybe<bool> EnableSsl;
    // Whether to use access token in auth options or not
    TMaybe<bool> UseAccessToken;
    // Whether to use default token file in auth options or not
    TMaybe<bool> UseDefaultTokenFile;
    // Whether to use IAM authentication (Yandex.Cloud) or not
    TMaybe<bool> UseIamAuth;
    // Whether to use static credentials (user/password) or not
    TMaybe<bool> UseStaticCredentials;
    // Whether to use OAuth 2.0 token exchange credentials or not
    TMaybe<bool> UseOauth2TokenExchange;
    // Whether to use export to YT command or not
    TMaybe<bool> UseExportToYt;
    // Whether to mention user account in --help command or not
    TMaybe<bool> MentionUserAccount;
    // A storage url to get latest YDB CLI version and to update from
    // If not set, than no updates nor latest version checks will be available
    std::optional<std::string> StorageUrl = std::nullopt;
    // Name of a directory in user home directory to save profile config
    TString YdbDir;
};

class TClientCommandRootCommon : public TClientCommandRootBase {
public:
    TClientCommandRootCommon(const TString& name, const TClientSettings& settings);
    void Config(TConfig& config) override;
    void ExtractParams(TConfig& config) override;
    void Parse(TConfig& config) override;
    void ParseCredentials(TConfig& config) override;
    void Validate(TConfig& config) override;
    int Run(TConfig& config) override;
protected:
    virtual void FillConfig(TConfig& config);
    virtual void SetCredentialsGetter(TConfig& config);

private:
    void ValidateSettings();

    void ParseProfile();
    void ParseAddress(TConfig&) override {}
    void ParseIamEndpoint(TConfig& config);
    void ParseCaCerts(TConfig& config) override;
    void ParseClientCert(TConfig& config) override;
    void ParseStaticCredentials(TConfig& config);
    static TString GetAddressFromString(const TString& address, bool* enableSsl = nullptr, std::vector<TString>* errors = nullptr);
    static bool ParseProtocolNoConfig(TString& address, bool* enableSsl, TString& message);
    bool TryGetParamFromProfile(const TString& name, const std::shared_ptr<IProfile>& profile, bool explicitOption,
                                std::function<bool(const TString&, const TString&, bool)> callback);

    // Gets more than one params from one profile source.
    // Returns true if at least one of the params are found in profile.
    bool TryGetParamsPackFromProfile(const std::shared_ptr<IProfile>& profile, bool explicitOption,
                                     std::function<bool(const TString& /*source*/, bool /*explicit*/, const std::vector<TString>& /*values*/)> callback,
                                     const std::initializer_list<TString>& names);

    TString Database;

    ui32 VerbosityLevel = 0;
    bool IsVerbose() const {
        return VerbosityLevel > 0;
    }

    TString ProfileName;
    TString ProfileFile;
    std::shared_ptr<IProfile> Profile;
    std::shared_ptr<IProfileManager> ProfileManager;

    TString UserName;
    TString PasswordFile;
    TString Password;
    // Password from separate option
    TString PasswordFileOption;
    TString PasswordOption;
    bool DoNotAskForPassword = false;

    const TClientSettings& Settings;
    TVector<TString> MisuseErrors;
};

}
}
