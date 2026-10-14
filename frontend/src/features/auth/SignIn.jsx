import bgImage from "../../assets/images/img-login.jpg";
import Input from "../../components/ui/Input";
<<<<<<< HEAD
import Button from "../../components/ui/Button";
=======
>>>>>>> origin/frontend

const SignIn = () => {
  return (
    <div
      className="flex flex-row items-center justify-center w-full min-h-screen px-20 py-10 bg-center bg-no-repeat bg-cover"
      style={{
        backgroundImage: `url("${bgImage}")`,
      }}
    >
      <h1 className="text-text-primary font-['Salsa',_sans-serif] text-[80px]">
        Ronde de sécurité mobile
      </h1>
      <div className="flex flex-col items-start justify-center gap-6 p-10 rounded-[20px] bg-surface/85">
        <h1 className="font-sans font-extrabold text-text-primary text-wrap text-[32px]">
          CONNEXION
        </h1>
        <p className="text-sm text-text-secondary">
          Connectez-vous à votre compte pour accéder à votre espace de
          supervision.
        </p>
        <Input
          label={"Nom d’utilisateur"}
          variant="defaulf"
          required={true}
          placeholder={"Ex:admin"}
        />
        <Input label={"Mot de passe"} variant="password" required={true} />
        <div className="flex flex-row items-center w-full">
          <span className="flex-1"></span>
          <p className="text-sm hover:text-primary-hover hover:cursor-pointer text-primary">
            Mot de passe oublié ?
          </p>
        </div>
<<<<<<< HEAD
        <Button
          onClick={console.log("Validate")}
          variant="primary"
          type="button"
          children={"Se connecter"}
          className="w-full"
        />
=======
        <button
          type="submit"
          className="w-full p-4 rounded-lg bg-primary text-surface"
        >
          Se connecter
        </button>
>>>>>>> origin/frontend
      </div>
    </div>
  );
};

export default SignIn;
