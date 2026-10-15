import Button from "../../../components/ui/Button";
import Input from "../../../components/ui/Input";
import AvatarUpload from "./AvatarUpload";

const Profile = () => {
  return (
    <section className="flex flex-col items-center flex-1 gap-4 p-4 rounded-lg bg-surface">
      <div className="w-full">
        <h1 className="font-sans text-xl font-semibold text-text-primary">
          Profile
        </h1>
      </div>
      <AvatarUpload size={150} />
      <form action="" className="flex flex-col justify-center w-full gap-4">
        <Input label={"Nom"} value={"Jhon Doe"} />
        <Input label={"Nom d'utilisateur"} value={"admin"} />
        <Input label={"Email"} value={" jhondoe@gmail.com"} />
        <Input label={"Téléphone"} value={"+261 34 10 16 86"} />
        <Input label={"Mot de passe"} value={"admin123"} variant="password" />
        <Input
          label={"Nouveau mot de passe"}
          placeholder={"Saissez nouveau mot de passe"}
          variant="password"
        />
        <div className="flex items-end justify-end w-full">
          <Button
            label="Enregistrer"
            variant="primary"
            disabled={true}
            children={"Enregistrer"}
          />
        </div>
      </form>
    </section>
  );
};

export default Profile;
