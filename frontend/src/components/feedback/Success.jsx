import SuccessIcon from "../../assets/icons/verify.svg?react";

const Success = ({ successMessage }) => {
  return (
    <div className="flex flex-row items-center justify-start w-auto gap-1 py-4 px-3 rounded-[20px] bg-surface/80">
      <SuccessIcon className={"h-7 w-7 text-success"} />
      <p>{successMessage}</p>
    </div>
  );
};

export default Success;
