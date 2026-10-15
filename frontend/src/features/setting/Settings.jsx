import Profile from "./layout/Profile";
import System from "./layout/System";

const Settings = () => {
  return (
    <div className="flex flex-row gap-6 ">
      <Profile />
      <System />
    </div>
  );
};

export default Settings;
