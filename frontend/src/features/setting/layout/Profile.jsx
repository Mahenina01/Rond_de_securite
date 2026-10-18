import { useCallback, useEffect, useState } from "react";
import toast from "react-hot-toast";

import Button from "../../../components/ui/Button";
import Input from "../../../components/ui/Input";
import AvatarUpload from "./AvatarUpload";
import authService from "../../../service/authService";

const emptyProfile = {
  photo: "",
  username: "",
  name: "",
  email: "",
};

const initialPassword = {
  currentPassword: "",
  newPassword: "",
};

const Profile = () => {
  const [formData, setFormData] = useState(emptyProfile);
  const [savedProfile, setSavedProfile] = useState(emptyProfile);
  const [passwordData, setPasswordData] = useState(initialPassword);

  const [loading, setLoading] = useState(true);
  const [savingProfile, setSavingProfile] = useState(false);
  const [savingPassword, setSavingPassword] = useState(false);

  // Charger le profil de l'utilisateur connecté
  const loadProfile = useCallback(async () => {
    setLoading(true);

    try {
      const response = await authService.getProfile();
      const profile = response?.data ?? response;

      const normalizedProfile = {
        photo: profile.photo ?? "",
        username: profile.username ?? "",
        name: profile.name ?? "",
        email: profile.email ?? "",
      };

      setFormData(normalizedProfile);
      setSavedProfile(normalizedProfile);
    } catch (error) {
      toast.error(
        error.response?.data?.message ?? "Impossible de charger le profil.",
      );
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    loadProfile();
  }, [loadProfile]);

  // Gérer les champs du profil
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

  // Gérer les champs du mot de passe
  const handlePasswordChange = (name) => (valueOrEvent) => {
    const value =
      typeof valueOrEvent === "string"
        ? valueOrEvent
        : (valueOrEvent?.target?.value ?? "");

    setPasswordData((previous) => ({
      ...previous,
      [name]: value,
    }));
  };

  const hasProfileChanges =
    JSON.stringify(formData) !== JSON.stringify(savedProfile);

  // Enregistrer les modifications du profil
  const handleProfileSubmit = async (event) => {
    event.preventDefault();

    if (!hasProfileChanges || savingProfile) return;

    const payload = {
      photo: formData.photo,
      username: formData.username.trim(),
      name: formData.name.trim(),
      email: formData.email.trim(),
    };

    setSavingProfile(true);

    try {
      const response = await authService.updateProfile(payload);
      const updatedProfile = response?.data ?? response;

      const normalizedProfile = {
        photo: updatedProfile.photo ?? payload.photo,
        username: updatedProfile.username ?? payload.username,
        name: updatedProfile.name ?? payload.name,
        email: updatedProfile.email ?? payload.email,
      };

      setFormData(normalizedProfile);
      setSavedProfile(normalizedProfile);

      toast.success("Profil mis à jour avec succès.");
    } catch (error) {
      toast.error(
        error.response?.data?.message ??
          "Erreur lors de la mise à jour du profil.",
      );
    } finally {
      setSavingProfile(false);
    }
  };

  // Changer le mot de passe
  const handlePasswordSubmit = async (event) => {
    event.preventDefault();

    if (savingPassword) return;

    if (!passwordData.currentPassword || !passwordData.newPassword) {
      toast.error("Veuillez renseigner les deux mots de passe.");
      return;
    }

    if (passwordData.newPassword.length < 8) {
      toast.error(
        "Le nouveau mot de passe doit contenir au moins 8 caractères.",
      );
      return;
    }

    setSavingPassword(true);

    try {
      await authService.changePassword({
        currentPassword: passwordData.currentPassword,
        newPassword: passwordData.newPassword,
      });

      setPasswordData(initialPassword);
      toast.success("Mot de passe modifié avec succès.");
    } catch (error) {
      toast.error(
        error.response?.data?.message ??
          "Impossible de modifier le mot de passe.",
      );
    } finally {
      setSavingPassword(false);
    }
  };

  if (loading) {
    return (
      <section className="flex items-center justify-center flex-1 p-6 rounded-lg bg-surface">
        <p className="text-text-secondary">Chargement du profil...</p>
      </section>
    );
  }

  return (
    <section className="flex flex-col flex-1 gap-6 p-4 rounded-lg bg-surface">
      <div>
        <h1 className="font-sans text-xl font-semibold text-text-primary">
          Profil
        </h1>
        <p className="mt-1 text-sm text-text-secondary">
          Gérez vos informations personnelles et votre sécurité.
        </p>
      </div>

      {/* Informations personnelles */}
      <form onSubmit={handleProfileSubmit} className="flex flex-col gap-4">
        <div className="flex justify-center">
          <AvatarUpload
            size={150}
            value={formData.photo}
            onChange={handleChangeFor("photo")}
          />
        </div>

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
          label="E-mail"
          name="email"
          inputType="email"
          value={formData.email}
          onChange={handleChangeFor("email")}
          required
        />

        <div className="flex justify-end">
          <Button
            type="submit"
            label={
              savingProfile ? "Enregistrement..." : "Enregistrer le profil"
            }
            variant="primary"
            disabled={!hasProfileChanges || savingProfile}
          >
            {savingProfile ? "Enregistrement..." : "Enregistrer le profil"}
          </Button>
        </div>
      </form>

      <div className="w-full h-px bg-border" />

      {/* Changement du mot de passe */}
      <form onSubmit={handlePasswordSubmit} className="flex flex-col gap-4">
        <div>
          <h2 className="text-lg font-semibold text-text-primary">Sécurité</h2>
          <p className="mt-1 text-sm text-text-secondary">
            Choisissez un nouveau mot de passe pour votre compte.
          </p>
        </div>

        <Input
          label="Mot de passe actuel"
          name="currentPassword"
          value={passwordData.currentPassword}
          onChange={handlePasswordChange("currentPassword")}
          variant="password"
          required
        />

        <Input
          label="Nouveau mot de passe"
          name="newPassword"
          value={passwordData.newPassword}
          onChange={handlePasswordChange("newPassword")}
          placeholder="8 caractères minimum"
          variant="password"
          required
        />

        <div className="flex justify-end">
          <Button
            type="submit"
            label={
              savingPassword ? "Modification..." : "Changer le mot de passe"
            }
            variant="primary"
            disabled={savingPassword}
          >
            {savingPassword ? "Modification..." : "Changer le mot de passe"}
          </Button>
        </div>
      </form>
    </section>
  );
};

export default Profile;
