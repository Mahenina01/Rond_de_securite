import { useState } from "react";
import Input from "../../components/ui/Input";
import Button from "../../components/ui/Button";
import ForgotPassword from "../../assets/icons/forgot-password.svg?react";
import { useNavigate } from "react-router";

export default function ForgotPasswordPage() {
  const [email, setEmail] = useState("");
  const [isSubmitting, setIsSubmitting] = useState(false);
  const navigate = useNavigate();

  const handleSubmit = (e) => {
    e.preventDefault();
    setIsSubmitting(true);

    console.log("Envoi du code de vérification à l'adresse :", email);

    setTimeout(() => {
      setIsSubmitting(false);
    }, 1500);
    navigate("/verify-email");
  };
  return (
    <main className="flex justify-center min-h-screen p-4 bg-background item-center">
      <div className="w-[522px] bg-surface rounded-[32px] px-[30px] py-[40px] shadow-[0_4px_200px_-3px_rgba(0,0,0,0.25)] flex flex-col gap-[24px]">
        <div className="flex justify-center py-2">
          <ForgotPassword className="w-[212px] h-[171px] object-contain text-primary" />
        </div>
        <div className="flex flex-col gap-2 text-center">
          <h1 className="text-xl font-black tracking-tight text-text-primary">
            MOT DE PASSE OUBLIÉ ?
          </h1>
          <p className="text-sm leading-relaxed text-text-secondary">
            Pas d'inquiétude. Saisissez votre adresse e-mail et nous vous
            enverrons un code de vérification pour réinitialiser votre mot de
            passe.
          </p>
        </div>
        <form onSubmit={handleSubmit} className="flex flex-col gap-[24px]">
          <Input
            variant="email"
            label="Adresse E-mail"
            name="email"
            placeholder="example0gmail.com"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            required={true}
          />
          <div className="flex items-center justify-between gap-4 pt-2">
            <Button
              type="button"
              variant="outline"
              className="justify-center w-1/2"
            >
              Retour
            </Button>
            <Button
              type="submit"
              variant="primary"
              state={isSubmitting ? "waiting" : "default"}
              disabled={!email || isSubmitting}
              className="justify-center w-1/2"
            >
              {isSubmitting ? "Envoi en cours..." : "Envoyer"}
            </Button>
          </div>
        </form>
      </div>
    </main>
  );
}
