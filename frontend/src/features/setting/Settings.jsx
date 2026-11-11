import Profile from "./layout/Profile";
import System from "./layout/System";
import { useOutletContext } from "react-router";

const Settings = () => {
  const {
    hasLogo,
    logoUrl,
    onLogoUpdated,
    onProfilePhotoUpdated,
  } = useOutletContext();

  return (
    <div className="flex min-w-0 flex-col items-stretch gap-4 xl:flex-row xl:gap-6">
      <Profile onProfilePhotoUpdated={onProfilePhotoUpdated} />
      <System
        hasLogo={hasLogo}
        logoUrl={logoUrl}
        onLogoUpdated={onLogoUpdated}
      />
    </div>
  );
};

export default Settings;
