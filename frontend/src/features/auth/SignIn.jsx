import { useState } from "react";
import smartCardImage from "../../assets/images/smart-card.webp";
import Input from "../../components/ui/Input";
import Button from "../../components/ui/Button";
import { useNavigate } from "react-router";
import authService from "../../service/authService";
import useLanguage from "../../context/useLanguage";

const SUPPORT_EMAIL = "avotranomena04@gmail.com";

const SignIn = () => {
  const navigate = useNavigate();
  const { t } = useLanguage();

  const [formData, setFormData] = useState({
    username: "",
    password: "",
  });

  const [formError, setFormError] = useState("");
  const [isLoading, setIsLoading] = useState(false);

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

  // Validation du formulaire
  const isFormValid =
    formData.username.trim() !== "" && formData.password.trim() !== "";
  const supportEmailHref = `mailto:${SUPPORT_EMAIL}?subject=${encodeURIComponent(
    t("Demande d’aide à la connexion"),
  )}&body=${encodeURIComponent(
    t(
      "Bonjour, je rencontre un problème pour me connecter à mon compte Rondix. Pouvez-vous m’aider ?",
    ),
  )}`;

  // Gestion de la connexion
  const handleSubmit = async (event) => {
    event.preventDefault();

    if (!isFormValid || isLoading) {
      setFormError(t("Veuillez renseigner tous les champs obligatoires."));
      return;
    }

    setFormError("");
    setIsLoading(true);

    try {
      const response = await authService.login({
        username: formData.username.trim(),
        password: formData.password,
      });

      console.log("Connexion réussie :", response);
      navigate("/");
    } catch (error) {
      const status = error.response?.status;

      if (status === 401 || status === 403) {
        setFormError(t("Nom d'utilisateur ou mot de passe incorrect."));
      } else {
        setFormError(
          error.response?.data?.message ||
            t("Impossible de se connecter au serveur. Veuillez réessayer."),
        );
      }
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div className="flex items-center justify-center w-full min-h-screen px-4 py-8 bg-background sm:px-6 lg:px-10">
      <div className="flex w-full max-w-md flex-col overflow-hidden rounded-[20px] bg-surface shadow-xl lg:max-w-5xl lg:flex-row lg:items-stretch">
        <div className="relative block h-[215px] w-full shrink-0 overflow-hidden lg:h-auto lg:min-h-[560px] lg:w-[46%]">
          <img
            src={smartCardImage}
            alt=""
            className="absolute inset-0 object-cover w-full h-full"
          />
          <div
            aria-hidden="true"
            className="absolute inset-0 bg-slate-950/40"
          />
          <div className="absolute inset-0 flex flex-col items-start justify-start p-6 text-left lg:items-center lg:justify-center lg:p-8 lg:text-center">
            <h2 className="font-sans text-3xl font-semibold tracking-tight text-white lg:text-4xl xl:text-5xl">
              {t("Bienvenue chez")}{" "}
              <span className="text-primary">{t("Rondix")}</span>
            </h2>
            <p className="mt-2 font-sans text-sm text-white/90 lg:hidden">
              {t("Connectez-vous pour continuer.")}
            </p>
            <p className="absolute hidden font-sans text-sm leading-6 inset-x-8 bottom-8 text-white/90 sm:text-base lg:block">
              {t("La vigilance avance. Vos rondes restent sous contrôle.")}
            </p>
          </div>
        </div>

        <div className="flex flex-col items-center justify-center w-full gap-6 p-8 sm:p-10 lg:flex-1 lg:px-12 xl:px-16">
          <h1 className="text-center font-sans text-[32px] font-extrabold text-text-primary">
            {t("CONNEXION")}
          </h1>

          <p className="text-sm text-center text-text-secondary">
            {t(
              "Connectez-vous à votre compte pour accéder à votre espace de supervision.",
            )}
          </p>

          <form onSubmit={handleSubmit} className="flex flex-col w-full gap-5">
            <Input
              label={t("Nom d'utilisateur")}
              name="username"
              variant="default"
              required
              placeholder={t("Ex. : admin")}
              value={formData.username}
              onChange={handleChangeFor("username")}
            />

            <Input
              label={t("Mot de passe")}
              name="password"
              variant="password"
              required
              placeholder={t("Saisissez votre mot de passe")}
              value={formData.password}
              onChange={handleChangeFor("password")}
            />

            <div className="flex flex-row items-center justify-end w-full">
              <button
                type="button"
                className="text-sm cursor-pointer text-primary hover:text-primary-hover"
                onClick={() => navigate("/forgot-password")}
              >
                {t("Mot de passe oublié ?")}
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
              disabled={!isFormValid || isLoading}
              className="w-full"
            >
              {isLoading ? t("Connexion en cours...") : t("Se connecter")}
            </Button>
          </form>

          <a
            href={supportEmailHref}
            className="text-sm text-center text-primary hover:text-primary-hover hover:underline focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary"
          >
            {t("Besoin d’aide pour vous connecter ?")}
          </a>
        </div>
      </div>
    </div>
  );
};

export default SignIn;
