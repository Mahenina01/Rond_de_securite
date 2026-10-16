import { useState } from "react";
import Button from "../../../components/ui/Button";
import Input from "../../../components/ui/Input";
import AvatarUpload from "./AvatarUpload";

const initialProfile = {
  name: "Jhon Doe",
  username: "admin",
  email: "jhondoe@gmail.com",
  phone: "+261 34 10 16 86",
  password: "admin123",
  newPassword: "",
};

const Profile = () => {
  const [formData, setFormData] = useState(initialProfile);

  // Détecter les changements dans les champs
  const handleChangeFor = (name) => (valueOrEvent) => {
    const value =
      typeof valueOrEvent === "string"
        ? valueOrEvent
        : (valueOrEvent?.target?.value ?? "");

    setFormData((previous) => ({
      ...previous,
      [name]: value,
    }));
  };

  // Vérifier si au moins une valeur a changé
  const hasChanges = Object.keys(initialProfile).some(
    (key) => formData[key] !== initialProfile[key],
  );

  // Enregistrer les modifications
  const handleSubmit = (event) => {
    event.preventDefault();

    if (!hasChanges) return;

    console.log("Profil modifié :", formData);

    // Après l'enregistrement, les valeurs deviennent la nouvelle référence.
    // Pour l'instant, aucune API n'est appelée.
  };

  return (
    <section className="flex flex-col items-center flex-1 gap-4 p-4 rounded-lg bg-surface">
      <div className="w-full">
        <h1 className="font-sans text-xl font-semibold text-text-primary">
          Profil
        </h1>
      </div>

      <AvatarUpload size={150} />

      <form
        onSubmit={handleSubmit}
        className="flex flex-col justify-center w-full gap-4"
      >
        <Input
          label="Nom"
          name="name"
          value={formData.name}
          onChange={handleChangeFor("name")}
          required
        />

        <Input
          label="Nom d'utilisateur"
          name="username"
          value={formData.username}
          onChange={handleChangeFor("username")}
          required
        />

        <Input
          label="Email"
          name="email"
          value={formData.email}
          onChange={handleChangeFor("email")}
          required
        />

        <Input
          label="Téléphone"
          name="phone"
          value={formData.phone}
          onChange={handleChangeFor("phone")}
          required
        />

        <Input
          label="Mot de passe actuel"
          name="password"
          value={formData.password}
          onChange={handleChangeFor("password")}
          variant="password"
          required
        />

        <Input
          label="Nouveau mot de passe"
          name="newPassword"
          value={formData.newPassword}
          onChange={handleChangeFor("newPassword")}
          placeholder="Saisissez un nouveau mot de passe"
          variant="password"
        />

        <div className="flex items-end justify-end w-full">
          <Button
            type="submit"
            label="Enregistrer"
            variant="primary"
            disabled={!hasChanges}
          >
            Enregistrer
          </Button>
        </div>
      </form>
    </section>
  );
};

export default Profile;
