import { useState } from "react";
import bgImage from "../../assets/images/img-login.jpg";
import Input from "../../components/ui/Input";
import Button from "../../components/ui/Button";
import { useNavigate } from "react-router";

const SignIn = () => {
  const navigate = useNavigate();

  const [formData, setFormData] = useState({
    username: "",
    password: "",
  });

  const [formError, setFormError] = useState("");

  // Gestion des champs du formulaire
  const handleChangeFor = (name) => (valueOrEvent) => {
    const value =
      typeof valueOrEvent === "string"
        ? valueOrEvent
        : (valueOrEvent?.target?.value ?? "");

    setFormData((previous) => ({
      ...previous,
      [name]: value,
    }));

    setFormError("");
  };

  // Vérifier si le formulaire est valide
  const isFormValid =
    formData.username.trim() !== "" && formData.password.trim() !== "";

  // Gestion de la connexion
  const handleSubmit = (event) => {
    event.preventDefault();

    if (!isFormValid) {
      setFormError("Veuillez renseigner tous les champs obligatoires.");
      return;
    }

    setFormError("");

    // À remplacer par un appel à l'API Spring Boot
    console.log("Données de connexion :", {
      username: formData.username.trim(),
      password: formData.password,
    });

    // Navigation temporaire, sans authentification réelle
    navigate("/");
  };

  return (
    <div
      className="flex flex-row items-center justify-center w-full min-h-screen gap-10 px-6 py-10 bg-center bg-no-repeat bg-cover lg:px-20"
      style={{
        backgroundImage: `url("${bgImage}")`,
      }}
    >
      <h1 className="hidden text-[80px] text-text-primary lg:block font-['Salsa',_sans-serif]">
        Ronde de sécurité mobile
      </h1>

      <div className="flex w-full max-w-md flex-col items-start justify-center gap-6 rounded-[20px] bg-surface/85 p-8 sm:p-10">
        <h1 className="font-sans text-[32px] font-extrabold text-text-primary">
          CONNEXION
        </h1>

        <p className="text-sm text-text-secondary">
          Connectez-vous à votre compte pour accéder à votre espace de
          supervision.
        </p>

        <form onSubmit={handleSubmit} className="flex flex-col w-full gap-5">
          <Input
            label="Nom d'utilisateur"
            name="username"
            variant="default"
            required
            placeholder="Ex. : admin"
            value={formData.username}
            onChange={handleChangeFor("username")}
          />

          <Input
            label="Mot de passe"
            name="password"
            variant="password"
            required
            placeholder="Saisissez votre mot de passe"
            value={formData.password}
            onChange={handleChangeFor("password")}
          />

          <div className="flex flex-row items-center justify-end w-full">
            <button
              type="button"
              className="text-sm cursor-pointer text-primary hover:text-primary-hover"
              onClick={() => navigate("/forgot-password")}
            >
              Mot de passe oublié ?
            </button>
          </div>

          {formError && (
            <p className="text-sm text-danger" role="alert">
              {formError}
            </p>
          )}

          <Button
            type="submit"
            variant="primary"
            disabled={!isFormValid}
            className="w-full"
          >
            Se connecter
          </Button>
        </form>
      </div>
    </div>
  );
};

export default SignIn;
